#include <w3copentracing/span_context.h>
#include <w3copentracing/tracer.h>

using namespace opentracing;
using namespace std;

namespace w3copentracing {

namespace {

template <class Carrier>
expected<void> InjectImpl(const opentracing::SpanContext& span_context,
                          Carrier& writer) {
  return {};
}

// template <class Carrier>
// static expected<void> InjectImpl(const PropagationOptions&
// propagation_options,
//                                  const SpanContext& span_context,
//                                  Carrier& writer) {
//   if (propagation_options.inject_error_code.value() != 0) {
//     return make_unexpected(propagation_options.inject_error_code);
//   }
//   auto mock_span_context = dynamic_cast<const
//   MockSpanContext*>(&span_context); if (mock_span_context == nullptr) {
//     return make_unexpected(invalid_span_context_error);
//   }
//   return mock_span_context->Inject(propagation_options, writer);
// }

template <class Carrier>
expected<unique_ptr<opentracing::SpanContext>> ExtractImpl(Carrier& reader) {
  return {};
}

// template <class Carrier>
// opentracing::expected<std::unique_ptr<opentracing::SpanContext>> ExtractImpl(
//     const PropagationOptions& propagation_options, Carrier& reader) {
//   if (propagation_options.extract_error_code.value() != 0) {
//     return
//     opentracing::make_unexpected(propagation_options.extract_error_code);
//   }
//   MockSpanContext* mock_span_context;
//   try {
//     mock_span_context = new MockSpanContext{};
//   } catch (const std::bad_alloc&) {
//     return opentracing::make_unexpected(
//         make_error_code(std::errc::not_enough_memory));
//   }
//   std::unique_ptr<opentracing::SpanContext> span_context(mock_span_context);
//   auto result = mock_span_context->Extract(propagation_options, reader);
//   if (!result) {
//     return opentracing::make_unexpected(result.error());
//   }
//   if (!*result) {
//     span_context.reset();
//   }
//   return std::move(span_context);
// }

}  // namespace

//   return std::unique_ptr<Span>{new MockSpan{shared_from_this(),
//   recorder_.get(),
//                                             operation_name, options}};
// } catch (const std::exception& e) {
//   fprintf(stderr, "Failed to start span: %s\n", e.what());
//   return nullptr;
// }

expected<void> Tracer::Inject(const opentracing::SpanContext& sc,
                              ostream& writer) const {
  return InjectImpl(sc, writer);
}

expected<void> Tracer::Inject(const opentracing::SpanContext& sc,
                              const TextMapWriter& writer) const {
  return InjectImpl(sc, writer);
}

expected<void> Tracer::Inject(const opentracing::SpanContext& sc,
                              const HTTPHeadersWriter& writer) const {
  return InjectImpl(sc, writer);
}

expected<unique_ptr<opentracing::SpanContext>> Tracer::Extract(
    istream& reader) const {
  return ExtractImpl(reader);
}

expected<unique_ptr<opentracing::SpanContext>> Tracer::Extract(
    const TextMapReader& reader) const {
  return ExtractImpl(reader);
}

expected<unique_ptr<opentracing::SpanContext>> Tracer::Extract(
    const HTTPHeadersReader& reader) const {
  return ExtractImpl(reader);
}

}  // namespace w3copentracing
