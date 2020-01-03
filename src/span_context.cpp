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
std::array<uint8_t, N> generate_random_bytes() {
  std::array<uint8_t, N> buf;
  using bytes_randomizer =
      std::independent_bits_engine<std::default_random_engine, CHAR_BIT,
                                   uint8_t>;
  std::random_device rdev{};
  std::default_random_engine reng{rdev()};
  bytes_randomizer bytes{reng};
  std::generate(std::begin(buf), std::end(buf), std::ref(bytes));
  return buf;
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

SpanContext::SpanContext(bool sampled)
    : SpanContext(generate_random_bytes<sizeof(TraceID)>(),
                  generate_random_bytes<sizeof(SpanID)>(), sampled) {}

SpanContext::SpanContext(const TraceID& trace_id, const SpanID& span_id,
                         bool sampled)
    : trace_id_{trace_id}, span_id_{span_id}, sampled_(sampled) {}

std::string SpanContext::ToTraceID() const noexcept {
  return to_hex(trace_id_);
}

std::string SpanContext::ToSpanID() const noexcept { return to_hex(span_id_); }

}  // namespace w3copentracing
