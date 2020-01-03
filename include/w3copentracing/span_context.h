#include <opentracing/span.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace w3copentracing {

class SpanContext : public opentracing::SpanContext {
 public:
  using TraceID = std::array<uint8_t, 16>;
  using SpanID = std::array<uint8_t, 8>;

  // Instantiate a SpanContext and generate trace and span IDs.
  SpanContext(bool sampled = true);

  // Instantiate a SpanContext using the passed trace and span IDs.
  SpanContext(const TraceID& trace_id, const SpanID& span_id,
              bool sampled = true);

  // Returns a HEX representation of trace ID.
  std::string ToTraceID() const noexcept override;

  // Returns a HEX representation of span ID.
  std::string ToSpanID() const noexcept override;

 protected:
  const TraceID trace_id_;
  const SpanID span_id_;
  bool sampled_;
};

}  // namespace w3copentracing
