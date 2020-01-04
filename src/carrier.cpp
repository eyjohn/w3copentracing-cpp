#include "carrier.h"

#include <opentracing/propagation.h>
#include <opentracing/util.h>
#include <w3copentracing/span_context.h>
#include <w3copentracing/tracer.h>

#include <iomanip>
#include <sstream>

using namespace opentracing;
using namespace std;

namespace w3copentracing {

namespace {

template <class Carrier>
expected<void> InjectImpl(const opentracing::SpanContext& span_context,
                          Carrier& writer) {
  auto ctx = dynamic_cast<const SpanContext*>(&span_context);
  if (ctx == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }

  ostringstream os_trace_parent;
  os_trace_parent << "00-" << ctx->ToTraceID() << "-" << ctx->ToSpanID() << "-"
                  << (ctx->sampled ? "01" : "00");
  auto res = writer.Set("trace-parent", os_trace_parent.str());
  if (!res) {
    return opentracing::make_unexpected(res.error());
  }

  ostringstream os_trace_state;
  bool has_baggage = false;
  span_context.ForeachBaggageItem([&](const string& key, const string& val) {
    if (has_baggage) {
      os_trace_state << ",";
    } else {
      has_baggage = true;
    }
    os_trace_state << key << "=" << val;
    return true;
  });

  if (has_baggage) {
    auto res2 = writer.Set("trace-state", os_trace_state.str());
    if (!res2) {
      return opentracing::make_unexpected(res2.error());
    }
  }
  return {};
}

template <std::size_t N>
std::array<uint8_t, N> read_hex(std::istream& istream) {
  std::array<uint8_t, N> res{};
  for (int i = 0; istream.good() && i < N; ++i) {
    char buf[3]{};
    int val;
    istream.get(buf, 3);
    istringstream bufstream{buf};
    bufstream >> std::hex >> val;
    if (bufstream.eof()) {
      res[i] = static_cast<uint8_t>(val);
    } else {
      istream.setstate(ios_base::failbit);
    }
  }
  return res;
}

bool parse_trace_parent(const std::string& input, SpanContext& output) {
  istringstream istream{input};
  const auto version = read_hex<1>(istream);
  if (version[0] != 0 || !istream.good() || istream.get() == '=') {
    return false;
  }

  output.trace_id = read_hex<16>(istream);
  if (!istream.good() || istream.get() == '=') {
    return false;
  }

  output.span_id = read_hex<8>(istream);
  if (!istream.good() || istream.get() == '=') {
    return false;
  }

  const auto flags = read_hex<1>(istream);
  if (flags[0] > uint8_t(1)) {
    return false;
  }
  output.sampled = flags[0];
  return true;
}

bool parse_trace_state(const std::string& input, SpanContext& output) {
  istringstream istream{input};
  string key, val;
  while (getline(istream, key, '=') && getline(istream, val, ',')) {
    output.baggage[key] = val;
  }
  return true;
}

template <class Carrier>
expected<unique_ptr<opentracing::SpanContext>> ExtractImpl(Carrier& reader) {
  auto trace_parent_maybe = reader.LookupKey("trace-parent");
  SpanContext out;
  if (trace_parent_maybe) {
    if (!parse_trace_parent(*trace_parent_maybe, out)) {
      return make_unexpected(span_context_corrupted_error);
    }
    auto trace_state_maybe = reader.LookupKey("trace-state");
    if (trace_state_maybe) {
      if (!parse_trace_state(*trace_state_maybe, out)) {
        return make_unexpected(span_context_corrupted_error);
      }
    } else if (!are_errors_equal(trace_state_maybe.error(),
                                 key_not_found_error)) {
      return make_unexpected(trace_state_maybe.error());
    }
  } else if (are_errors_equal(trace_parent_maybe.error(),
                              key_not_found_error)) {
    return {unique_ptr<opentracing::SpanContext>{}};
  } else if (are_errors_equal(trace_parent_maybe.error(),
                              lookup_key_not_supported_error)) {
    std::string trace_parent, trace_state;
    auto res = reader.ForeachKey([&](auto key, auto val) -> expected<void> {
      if (key == "trace-parent") {
        trace_parent = val;
      } else if (key == "trace-state") {
        trace_state = val;
      }
      return {};
    });
    if (!res) {
      return make_unexpected(res.error());
    }
    if (trace_parent.empty()) {
      return {unique_ptr<opentracing::SpanContext>{}};
    }
    if (!parse_trace_parent(trace_parent, out) ||
        (!trace_state.empty() && !parse_trace_state(trace_state, out))) {
      return make_unexpected(span_context_corrupted_error);
    }
  } else {
    return make_unexpected(trace_parent_maybe.error());
  }
  return {unique_ptr<opentracing::SpanContext>{new SpanContext(out)}};
}

}  // namespace

expected<void> Carrier::Inject(const opentracing::SpanContext& span_context,
                               const TextMapWriter& writer) {
  return InjectImpl(span_context, writer);
}

expected<void> Carrier::Inject(const opentracing::SpanContext& span_context,
                               const HTTPHeadersWriter& writer) {
  return InjectImpl(span_context, writer);
}

expected<unique_ptr<opentracing::SpanContext>> Carrier::Extract(
    const TextMapReader& reader) {
  return ExtractImpl(reader);
}

expected<unique_ptr<opentracing::SpanContext>> Carrier::Extract(
    const HTTPHeadersReader& reader) {
  return ExtractImpl(reader);
}

}  // namespace w3copentracing
