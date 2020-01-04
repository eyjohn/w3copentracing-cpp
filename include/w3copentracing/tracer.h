#pragma once

#include <opentracing/propagation.h>
#include <opentracing/span.h>
#include <opentracing/string_view.h>
#include <opentracing/tracer.h>
#include <w3copentracing/span_context.h>

#include <memory>
#include <string>

namespace w3copentracing {

class Tracer : public opentracing::Tracer {
 public:
  using opentracing::Tracer::Extract;
  using opentracing::Tracer::Inject;

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& sc,
      const opentracing::TextMapWriter& writer) const override;

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& sc,
      const opentracing::HTTPHeadersWriter& writer) const override;

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::TextMapReader& reader) const override;

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::HTTPHeadersReader& reader) const override;
};

}  // namespace w3copentracing