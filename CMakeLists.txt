cmake_minimum_required(VERSION 3.16)

project(w3copentracing VERSION 1.0.0 DESCRIPTION "A partial implementation of an OpenTracing tracer with W3C compliance.")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(w3copentracing STATIC src/tracer.cpp)
set_target_properties(w3copentracing PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "include/w3copentracing/tracer.h")

configure_file(w3copentracing.pc.in w3copentracing.pc @ONLY)

install(TARGETS w3copentracing
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/w3copentracing)

install(FILES ${CMAKE_BINARY_DIR}/w3copentracing.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)


enable_testing()

add_subdirectory(test)
