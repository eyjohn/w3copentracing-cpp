#include <gmock/gmock.h>
#include <opentracing/propagation.h>

class MockHTTPHeadersWriter : public opentracing::HTTPHeadersWriter {
 public:
  MOCK_CONST_METHOD2(Set,
                     opentracing::expected<void>(opentracing::string_view,
                                                 opentracing::string_view));
};

class MockTextMapWriter : public opentracing::TextMapWriter {
 public:
  MOCK_CONST_METHOD2(Set,
                     opentracing::expected<void>(opentracing::string_view,
                                                 opentracing::string_view));
};

class MockHTTPHeadersReader : public opentracing::HTTPHeadersReader {
 public:
  MOCK_CONST_METHOD1(LookupKey, opentracing::expected<opentracing::string_view>(
                                    opentracing::string_view));
  MOCK_CONST_METHOD1(
      ForeachKey,
      opentracing::expected<void>(
          std::function<opentracing::expected<void>(
              opentracing::string_view, opentracing::string_view)>));
};

class MockTextMapReader : public opentracing::TextMapReader {
 public:
  MOCK_CONST_METHOD1(LookupKey, opentracing::expected<opentracing::string_view>(
                                    opentracing::string_view));
  MOCK_CONST_METHOD1(
      ForeachKey,
      opentracing::expected<void>(
          std::function<opentracing::expected<void>(
              opentracing::string_view, opentracing::string_view)>));
};
