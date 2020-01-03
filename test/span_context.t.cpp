#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test_span_context.h"

using namespace std;
using namespace w3copentracing;
using namespace testing;

TEST(SpanContext, Instantiate) {
  auto rand_span = TestSpanContext();
  // Non deterministic checks, statistically unlikely to fail
  EXPECT_THAT(rand_span.ToTraceID(), Ne("00000000000000000000000000000000"));
  EXPECT_THAT(rand_span.ToSpanID(), Ne("0000000000000000"));
  EXPECT_THAT(rand_span.sampled(), Eq(true));

  auto rand_span_other = TestSpanContext();
  // Non deterministic checks, statistically unlikely to fail
  EXPECT_THAT(rand_span_other.ToTraceID(), Ne(rand_span.ToTraceID()));
  EXPECT_THAT(rand_span_other.ToSpanID(), Ne(rand_span.ToSpanID()));

  auto span = TestSpanContext(SpanContext::TraceID{1}, SpanContext::SpanID{2});
  EXPECT_THAT(span.ToTraceID(), Eq("01000000000000000000000000000000"));
  EXPECT_THAT(span.ToSpanID(), Eq("0200000000000000"));
  EXPECT_THAT(span.sampled(), Eq(true));

  auto unsampled_span = TestSpanContext(false);
  EXPECT_THAT(unsampled_span.sampled(), Eq(false));
}
