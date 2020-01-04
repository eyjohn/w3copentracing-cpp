#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentracing/propagation.h>
#include <opentracing/string_view.h>
#include <w3copentracing/span_context.h>

#include "carrier.h"

using namespace std;
using namespace w3copentracing;
using namespace testing;
namespace ot = opentracing;

class MockHTTPHeadersWriter : public ot::HTTPHeadersWriter {
 public:
  MOCK_CONST_METHOD2(Set, ot::expected<void>(ot::string_view, ot::string_view));
};

class MockTextMapWriter : public ot::TextMapWriter {
 public:
  MOCK_CONST_METHOD2(Set, ot::expected<void>(ot::string_view, ot::string_view));
};

template <typename T>
class CarrierTypedWriter : public Test {};
using WriterTypes = Types<MockHTTPHeadersWriter, MockTextMapWriter>;
TYPED_TEST_SUITE(CarrierTypedWriter, WriterTypes);

TYPED_TEST(CarrierTypedWriter, InjectBasicSpan) {
  SpanContext span_context{SpanContext::TraceID{1}, SpanContext::SpanID{2}};
  TypeParam mock_writer;
  EXPECT_CALL(mock_writer,
              Set({"trace-parent"},
                  {"00-01000000000000000000000000000000-0200000000000000-01"}))
      .WillOnce(Return(ot::expected<void>{}));
  Carrier::Inject(span_context, mock_writer);
}

TYPED_TEST(CarrierTypedWriter, InjectUnsampledSpan) {
  SpanContext span_context{SpanContext::TraceID{1}, SpanContext::SpanID{2},
                           false};
  TypeParam mock_writer;
  EXPECT_CALL(mock_writer,
              Set({"trace-parent"},
                  {"00-01000000000000000000000000000000-0200000000000000-00"}))
      .WillOnce(Return(ot::expected<void>{}));
  Carrier::Inject(span_context, mock_writer);
}

TYPED_TEST(CarrierTypedWriter, InjectSpanWithBaggage) {
  SpanContext span_context{SpanContext::TraceID{1},
                           SpanContext::SpanID{2},
                           true,
                           {{"key1", "val1"}, {"key2", "val2"}}};
  TypeParam mock_writer;
  EXPECT_CALL(mock_writer,
              Set({"trace-parent"},
                  {"00-01000000000000000000000000000000-0200000000000000-01"}))
      .WillOnce(Return(ot::expected<void>{}));
  EXPECT_CALL(mock_writer, Set({"trace-state"}, {"key1=val1,key2=val2"}))
      .WillOnce(Return(ot::expected<void>{}));
  Carrier::Inject(span_context, mock_writer);
}

class MockHTTPHeadersReader : public ot::HTTPHeadersReader {
 public:
  MOCK_CONST_METHOD1(LookupKey, ot::expected<ot::string_view>(ot::string_view));
  MOCK_CONST_METHOD1(
      ForeachKey,
      ot::expected<void>(
          std::function<ot::expected<void>(ot::string_view, ot::string_view)>));
};

class MockTextMapReader : public ot::TextMapReader {
 public:
  MOCK_CONST_METHOD1(LookupKey, ot::expected<ot::string_view>(ot::string_view));
  MOCK_CONST_METHOD1(
      ForeachKey,
      ot::expected<void>(
          std::function<ot::expected<void>(ot::string_view, ot::string_view)>));
};

template <typename T>
class CarrierTypedReader : public Test {};
using ReaderTypes = Types<MockHTTPHeadersReader, MockTextMapReader>;
TYPED_TEST_SUITE(CarrierTypedReader, ReaderTypes);

TYPED_TEST(CarrierTypedReader, ExtractNoSpan) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(Return(ot::make_unexpected(ot::key_not_found_error)));

  auto maybe_span_context_ptr = Carrier::Extract(mock_reader);
  ASSERT_TRUE(maybe_span_context_ptr);
  EXPECT_THAT(maybe_span_context_ptr->get(), Eq(nullptr));
}

TYPED_TEST(CarrierTypedReader, ExtractBasicSpan) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(Return(ot::expected<ot::string_view>{
          "00-01000000000000000000000000000000-0200000000000000-01"}));
  EXPECT_CALL(mock_reader, LookupKey({"trace-state"}))
      .WillOnce(Return(ot::make_unexpected(ot::key_not_found_error)));

  auto maybe_span_context_ptr = Carrier::Extract(mock_reader);
  ASSERT_TRUE(maybe_span_context_ptr);
  auto span_context_ptr =
      dynamic_cast<SpanContext*>(maybe_span_context_ptr->get());
  ASSERT_TRUE(span_context_ptr);
  EXPECT_THAT(*span_context_ptr,
              Eq(SpanContext{SpanContext::TraceID{1}, SpanContext::SpanID{2}}));
}

TYPED_TEST(CarrierTypedReader, ExtractUnsampledSpan) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(Return(ot::expected<ot::string_view>{
          "00-01000000000000000000000000000000-0200000000000000-00"}));
  EXPECT_CALL(mock_reader, LookupKey({"trace-state"}))
      .WillOnce(Return(ot::make_unexpected(ot::key_not_found_error)));

  auto maybe_span_context_ptr = Carrier::Extract(mock_reader);
  ASSERT_TRUE(maybe_span_context_ptr);
  auto span_context_ptr =
      dynamic_cast<SpanContext*>(maybe_span_context_ptr->get());
  ASSERT_TRUE(span_context_ptr);
  EXPECT_THAT(
      *span_context_ptr,
      Eq(SpanContext{SpanContext::TraceID{1}, SpanContext::SpanID{2}, false}));
}

TYPED_TEST(CarrierTypedReader, ExtractSpanWithBaggage) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(Return(ot::expected<ot::string_view>{
          "00-01000000000000000000000000000000-0200000000000000-01"}));
  EXPECT_CALL(mock_reader, LookupKey({"trace-state"}))
      .WillOnce(Return(ot::expected<ot::string_view>{"key1=val1,key2=val2"}));

  auto maybe_span_context_ptr = Carrier::Extract(mock_reader);
  ASSERT_TRUE(maybe_span_context_ptr);
  auto span_context_ptr =
      dynamic_cast<SpanContext*>(maybe_span_context_ptr->get());
  ASSERT_TRUE(span_context_ptr);
  EXPECT_THAT(*span_context_ptr,
              Eq(SpanContext{SpanContext::TraceID{1},
                             SpanContext::SpanID{2},
                             true,
                             {{"key1", "val1"}, {"key2", "val2"}}}));
}

TYPED_TEST(CarrierTypedReader, ExtractFallbackToForeach) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(
          Return(ot::make_unexpected(ot::lookup_key_not_supported_error)));

  EXPECT_CALL(mock_reader, ForeachKey(_))
      .WillOnce(Invoke([](auto cb) -> ot::expected<void> {
        cb({"trace-parent"},
           {"00-01000000000000000000000000000000-0200000000000000-01"});
        cb({"trace-state"}, {"key1=val1,key2=val2"});
        return {};
      }));

  auto maybe_span_context_ptr = Carrier::Extract(mock_reader);
  ASSERT_TRUE(maybe_span_context_ptr);
  auto span_context_ptr =
      dynamic_cast<SpanContext*>(maybe_span_context_ptr->get());
  ASSERT_TRUE(span_context_ptr);
  EXPECT_THAT(*span_context_ptr,
              Eq(SpanContext{SpanContext::TraceID{1},
                             SpanContext::SpanID{2},
                             true,
                             {{"key1", "val1"}, {"key2", "val2"}}}));
}
