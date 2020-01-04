#include "carrier.h"

#include <w3copentracing/span_context.h>
#include <w3copentracing/tracer.h>

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
  writer.Set("trace-parent", os_trace_parent.str());

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
    writer.Set("trace-state", os_trace_state.str());
  }
  return {};
}

template <class Carrier>
expected<unique_ptr<opentracing::SpanContext>> ExtractImpl(Carrier& reader) {
  return {};
}

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
  return {};
}

expected<unique_ptr<opentracing::SpanContext>> Carrier::Extract(
    const HTTPHeadersReader& reader) {
  return Extract(dynamic_cast<const TextMapReader&>(reader));
}

}  // namespace
