cmake_minimum_required(VERSION 3.10)

project(w3copentracing VERSION 1.0.0 DESCRIPTION "A partial implementation of an OpenTracing tracer with W3C compliance.")

include(GNUInstallDirs)

include_directories(include)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false src/*.h src/*.cpp)
file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false include/*.h)

find_package(OpenTracing REQUIRED)

add_library(w3copentracing STATIC ${SOURCES})
set_target_properties(w3copentracing PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${HEADERS}"
    POSITION_INDEPENDENT_CODE ON)

configure_file(w3copentracing.pc.in w3copentracing.pc @ONLY)

install(TARGETS w3copentracing
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/w3copentracing)

install(FILES ${CMAKE_BINARY_DIR}/w3copentracing.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

enable_testing()

add_subdirectory(test)
