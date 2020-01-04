#pragma once

#include <opentracing/propagation.h>
#include <opentracing/span.h>

#include <memory>
#include <string>

namespace w3copentracing {

class Carrier {
 public:
  static opentracing::expected<void> Inject(
      const opentracing::SpanContext& sc,
      const opentracing::TextMapWriter& writer);

  static opentracing::expected<void> Inject(
      const opentracing::SpanContext& sc,
      const opentracing::HTTPHeadersWriter& writer);

  static opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
  Extract(const opentracing::TextMapReader& reader);

  static opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
  Extract(const opentracing::HTTPHeadersReader& reader);
};

}  // namespace w3copentracing