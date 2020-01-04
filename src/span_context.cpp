#include <w3copentracing/span_context.h>

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

using namespace opentracing;
using namespace std;

namespace w3copentracing {

namespace {

template <std::size_t N>
void generate_random_bytes(std::array<uint8_t, N>& target) {
  using bytes_randomizer =
      std::independent_bits_engine<std::default_random_engine, CHAR_BIT,
                                   uint8_t>;
  std::random_device rdev{};
  std::default_random_engine reng{rdev()};
  bytes_randomizer bytes{reng};
  std::generate(std::begin(target), std::end(target), std::ref(bytes));
}

template <std::size_t N>
std::string to_hex(const std::array<uint8_t, N>& data) {
  std::ostringstream ss;
  ss << std::hex << std::uppercase << std::setfill('0');
  for (auto byte : data) {
    ss << std::setw(2) << static_cast<int>(byte);
  }
  return ss.str();
}

}  // namespace

SpanContext SpanContext::Generate(bool sampled, const Baggage& baggage) {
  SpanContext ctx{{}, {}, sampled, baggage};
  do {
    generate_random_bytes(ctx.trace_id);
  } while (ctx.trace_id == TraceID{});
  do {
    generate_random_bytes(ctx.span_id);
  } while (ctx.span_id == SpanID{});
  return ctx;
}

SpanContext::SpanContext(const TraceID& trace_id_, const SpanID& span_id_,
                         bool sampled_, const SpanContext::Baggage& baggage_)
    : trace_id(trace_id_),
      span_id(span_id_),
      sampled(sampled_),
      baggage(baggage_) {}

std::unique_ptr<opentracing::SpanContext> SpanContext::Clone() const noexcept {
  return std::unique_ptr<opentracing::SpanContext>{new SpanContext{*this}};
}

std::string SpanContext::ToTraceID() const noexcept { return to_hex(trace_id); }

std::string SpanContext::ToSpanID() const noexcept { return to_hex(span_id); }

bool SpanContext::operator==(const SpanContext& other) const {
  return trace_id == other.trace_id && span_id == other.span_id &&
         sampled == other.sampled && baggage == other.baggage;
}

void SpanContext::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  for (const auto& kv : baggage) {
    if (!f(kv.first, kv.second)) {
      return;
    }
  }
}

}  // namespace w3copentracing
