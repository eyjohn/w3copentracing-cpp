#pragma once

#include <opentracing/span.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace w3copentracing {

class SpanContext : public opentracing::SpanContext {
 public:
  using TraceID = std::array<uint8_t, 16>;
  using SpanID = std::array<uint8_t, 8>;
  using Baggage = std::map<std::string, std::string>;

  // Generate a SpanID with randomised bytes
  static SpanID GenerateSpanID();

  // Generate a SpanID with randomised bytes
  static TraceID GenerateTraceID();

  // Make a copy of this context
  std::unique_ptr<opentracing::SpanContext> Clone() const noexcept override;

  // Initialise a SpanContext
  SpanContext(const TraceID& trace_id = {}, const SpanID& span_id = {},
              bool sampled = true, const SpanContext::Baggage& baggage = {});

  // Returns a HEX representation of trace ID.
  std::string ToTraceID() const noexcept override;

  // Returns a HEX representation of span ID.
  std::string ToSpanID() const noexcept override;

  bool operator==(const SpanContext& other) const;

  // Iterate over all baggage items and invoke the passed function for each.
  void ForeachBaggageItem(
      std::function<bool(const std::string& key, const std::string& value)> f)
      const override;

  // Publically accessible data members
  TraceID trace_id;
  SpanID span_id;
  bool sampled;
  Baggage baggage;
};

}  // namespace w3copentracing
