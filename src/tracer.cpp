#include <w3copentracing/tracer.h>

#include "carrier.h"

using namespace opentracing;
using namespace std;

namespace w3copentracing {

expected<void> Tracer::Inject(const opentracing::SpanContext& span_context,
                              const TextMapWriter& writer) const {
  return Carrier::Inject(span_context, writer);
}

expected<void> Tracer::Inject(const opentracing::SpanContext& span_context,
                              const HTTPHeadersWriter& writer) const {
  return Carrier::Inject(span_context, writer);
}

expected<unique_ptr<opentracing::SpanContext>> Tracer::Extract(
    const TextMapReader& reader) const {
  return Carrier::Extract(reader);
}

expected<unique_ptr<opentracing::SpanContext>> Tracer::Extract(
    const HTTPHeadersReader& reader) const {
  return Carrier::Extract(reader);
}

}  // namespace w3copentracing
