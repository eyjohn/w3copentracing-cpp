#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <w3copentracing/span_context.h>

#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace w3copentracing;
using namespace testing;

TEST(SpanContext, Instantiate) {
  auto span = SpanContext{
      SpanContext::TraceID{1}, SpanContext::SpanID{2}, false, {{"key", "val"}}};
  EXPECT_THAT(span.trace_id, Eq(SpanContext::TraceID{1}));
  EXPECT_THAT(span.span_id, Eq(SpanContext::SpanID{2}));
  EXPECT_THAT(span.sampled, Eq(false));
  EXPECT_THAT(span.baggage, Eq(SpanContext::Baggage{{"key", "val"}}));
}

TEST(SpanContext, InstantiateDefault) {
  auto span = SpanContext{};
  EXPECT_THAT(span.trace_id, Eq(SpanContext::TraceID{}));
  EXPECT_THAT(span.span_id, Eq(SpanContext::SpanID{}));
  EXPECT_THAT(span.sampled, Eq(true));
  EXPECT_THAT(span.baggage, Eq(SpanContext::Baggage{}));
}

TEST(SpanContext, GenerateSpanID) {
  auto id = SpanContext::GenerateSpanID();
  EXPECT_THAT(id, Ne(SpanContext::SpanID{}));
}

TEST(SpanContext, GenerateTraceID) {
  auto id = SpanContext::GenerateTraceID();
  EXPECT_THAT(id, Ne(SpanContext::TraceID{}));
}

TEST(SpanContext, ToTraceID) {
  auto span = SpanContext{SpanContext::TraceID{1}};
  EXPECT_THAT(span.ToTraceID(), Eq("01000000000000000000000000000000"));
}

TEST(SpanContext, ToSpanID) {
  auto span = SpanContext{{}, SpanContext::SpanID{2}};
  EXPECT_THAT(span.ToSpanID(), Eq("0200000000000000"));
}

TEST(SpanContext, Clone) {
  auto span = SpanContext{
      SpanContext::TraceID{1}, SpanContext::SpanID{2}, false, {{"key", "val"}}};
  auto span2 = span.Clone();
  EXPECT_THAT(dynamic_cast<SpanContext&>(*span2), Eq(span));
}

TEST(SpanContext, ForeachBaggageItem) {
  using Observed = vector<pair<string, string>>;
  Observed all, first;
  auto span = SpanContext{{}, {}, {}, {{"key1", "val1"}, {"key2", "val2"}}};

  span.ForeachBaggageItem([&](const string& key, const string& val) {
    all.push_back({key, val});
    return true;
  });
  EXPECT_THAT(all, Eq(Observed{{"key1", "val1"}, {"key2", "val2"}}));

  span.ForeachBaggageItem([&](const string& key, const string& val) {
    first.push_back({key, val});
    return false;
  });
  EXPECT_THAT(first, Eq(Observed{{"key1", "val1"}}));
}
