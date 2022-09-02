project(httplib)

file(GLOB HTTPLIB_HEADERS include/httplib/httplib.hpp)

add_library(httplib INTERFACE)
add_library(httplib::httplib ALIAS httplib)

target_include_directories(httplib
    INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)