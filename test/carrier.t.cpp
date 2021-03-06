#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentracing/propagation.h>
#include <opentracing/string_view.h>
#include <w3copentracing/span_context.h>

#include "carrier.h"
#include "mock_writers.h"

using namespace std;
using namespace w3copentracing;
using namespace testing;
namespace ot = opentracing;

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

TYPED_TEST(CarrierTypedWriter, InjectPropagateFail) {
  SpanContext span_context{};
  TypeParam mock_writer;
  EXPECT_CALL(mock_writer, Set(_, _))
      .WillOnce(Return(ot::make_unexpected(ot::invalid_carrier_error)));
  auto res = Carrier::Inject(span_context, mock_writer);
  ASSERT_FALSE(res);
  EXPECT_THAT(res.error(), Eq(ot::invalid_carrier_error));
}

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

TYPED_TEST(CarrierTypedReader, ExtractPropagateFail) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey(_))
      .WillOnce(Return(ot::make_unexpected(ot::invalid_carrier_error)));

  auto res = Carrier::Extract(mock_reader);
  ASSERT_FALSE(res);
  EXPECT_THAT(res.error(), Eq(ot::invalid_carrier_error));
}

TYPED_TEST(CarrierTypedReader, ExtractForeachKeyPropagateFail) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(
          Return(ot::make_unexpected(ot::lookup_key_not_supported_error)));
  EXPECT_CALL(mock_reader, ForeachKey(_))
      .WillOnce(Return(ot::make_unexpected(ot::invalid_carrier_error)));
  auto res = Carrier::Extract(mock_reader);
  ASSERT_FALSE(res);
  EXPECT_THAT(res.error(), Eq(ot::invalid_carrier_error));
}

TYPED_TEST(CarrierTypedReader, ExtractFail) {
  TypeParam mock_reader;
  EXPECT_CALL(mock_reader, LookupKey({"trace-parent"}))
      .WillOnce(Return(ot::expected<ot::string_view>{"01"}));
  auto res = Carrier::Extract(mock_reader);
  ASSERT_FALSE(res);
  EXPECT_THAT(res.error(), Eq(ot::span_context_corrupted_error));
}
