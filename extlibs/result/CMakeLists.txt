project(result)

file(GLOB RESULT_HEADERS include/result/result.hpp)

add_library(result INTERFACE)
add_library(result::result ALIAS httplib)

target_include_directories(result
    INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)