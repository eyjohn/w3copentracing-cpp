find_package(GTest REQUIRED)
include(GoogleTest)

set(BINARY ${CMAKE_PROJECT_NAME}_test)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)

add_executable(${BINARY} ${TEST_SOURCES})

target_include_directories(${BINARY} PRIVATE ../src)
target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME} OpenTracing::opentracing GTest::GTest GTest::Main gmock)

gtest_add_tests(TARGET ${BINARY})
