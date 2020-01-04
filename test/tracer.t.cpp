#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentracing/propagation.h>
#include <w3copentracing/tracer.h>

#include "mock_writers.h"

using namespace std;
using namespace w3copentracing;
using namespace testing;
namespace ot = opentracing;

class ConcreteStubTracer : public w3copentracing::Tracer {
 public:
  std::unique_ptr<ot::Span> StartSpanWithOptions(
      ot::string_view operation_name, const ot::StartSpanOptions& options) const
      noexcept override {
    return {};
  }

  using w3copentracing::Tracer::Extract;
  using w3copentracing::Tracer::Inject;

  ot::expected<void> Inject(const ot::SpanContext& sc,
                            std::ostream& writer) const override {
    return {};
  }

  ot::expected<std::unique_ptr<ot::SpanContext>> Extract(
      std::istream& reader) const override {
    return {};
  }
};

template <typename T>
class TracerTypedWriter : public Test {};
using WriterTypes = Types<MockHTTPHeadersWriter, MockTextMapWriter>;
TYPED_TEST_SUITE(TracerTypedWriter, WriterTypes);

TYPED_TEST(TracerTypedWriter, InjectBasicSpan) {
  ConcreteStubTracer tracer;
  SpanContext span_context{SpanContext::TraceID{1}, SpanContext::SpanID{2}};
  TypeParam mock_writer;
  EXPECT_CALL(mock_writer,
              Set({"trace-parent"},
                  {"00-01000000000000000000000000000000-0200000000000000-01"}))
      .WillOnce(Return(ot::expected<void>{}));
  tracer.Inject(span_context, mock_writer);
}

template <typename T>
class TracerTypedReader : public Test {};
using ReaderTypes = Types<MockHTTPHeadersReader, MockTextMapReader>;
TYPED_TEST_SUITE(TracerTypedReader, ReaderTypes);

TYPED_TEST(TracerTypedReader, ExtractBasicSpan) {
  ConcreteStubTracer tracer;
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(Return(ot::expected<ot::string_view>{
          "00-01000000000000000000000000000000-0200000000000000-01"}));
  EXPECT_CALL(mock_reader, LookupKey({"trace-state"}))
      .WillOnce(Return(ot::make_unexpected(ot::key_not_found_error)));

  auto maybe_span_context_ptr = tracer.Extract(mock_reader);
  ASSERT_TRUE(maybe_span_context_ptr);
  auto span_context_ptr =
      dynamic_cast<SpanContext*>(maybe_span_context_ptr->get());
  ASSERT_TRUE(span_context_ptr);
  EXPECT_THAT(*span_context_ptr,
              Eq(SpanContext{SpanContext::TraceID{1}, SpanContext::SpanID{2}}));
}
