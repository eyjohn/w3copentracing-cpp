#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <w3copentracing/span_context.h>

#include <functional>
#include <map>
#include <string>

using namespace std;
using namespace w3copentracing;
using namespace testing;

class MockSpanContext : public SpanContext {
 public:
  template <class... T>
  MockSpanContext(T... rest) : SpanContext(rest...) {}

  template <class... T>
  MockSpanContext(const map<string, string>& baggage, T... rest)
      : baggage_(baggage), SpanContext(rest...) {}

  std::unique_ptr<opentracing::SpanContext> Clone() const noexcept {
    return {};
  }
  void ForeachBaggageItem(
      std::function<bool(const std::string& key, const std::string& value)> f)
      const {
    for (const auto& kv : baggage_) {
      f(kv.first, kv.second);
    }
  }
  bool sampled() const { return sampled_; }
  map<string, string> baggage_;
};

TEST(SpanContext, Instantiate) {
  auto rand_span = MockSpanContext();
  // Non deterministic checks, statistically unlikely to fail
  EXPECT_THAT(rand_span.ToTraceID(), Ne("00000000000000000000000000000000"));
  EXPECT_THAT(rand_span.ToSpanID(), Ne("0000000000000000"));
  EXPECT_THAT(rand_span.sampled(), Eq(true));

  auto rand_span_other = MockSpanContext();
  // Non deterministic checks, statistically unlikely to fail
  EXPECT_THAT(rand_span_other.ToTraceID(), Ne(rand_span.ToTraceID()));
  EXPECT_THAT(rand_span_other.ToSpanID(), Ne(rand_span.ToSpanID()));

  auto span = MockSpanContext(SpanContext::TraceID{1}, SpanContext::SpanID{2});
  EXPECT_THAT(span.ToTraceID(), Eq("01000000000000000000000000000000"));
  EXPECT_THAT(span.ToSpanID(), Eq("0200000000000000"));
  EXPECT_THAT(span.sampled(), Eq(true));

  auto unsampled_span = MockSpanContext(false);
  EXPECT_THAT(unsampled_span.sampled(), Eq(false));
}
