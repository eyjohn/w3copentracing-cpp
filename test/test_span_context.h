#include <w3copentracing/span_context.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

class TestSpanContext : public w3copentracing::SpanContext {
 public:
  template <class... T>
  TestSpanContext(T... rest) : SpanContext(rest...) {}

  template <class... T>
  TestSpanContext(const std::map<std::string, std::string>& baggage, T... rest)
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
  std::map<std::string, std::string> baggage_;
};