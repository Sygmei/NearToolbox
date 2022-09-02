project(nlohmann)

file(GLOB_RECURSE NLOHMANN_HEADERS include/nlohmann/*.hpp)

add_library(nlohmann INTERFACE)
add_library(nlohmann::json ALIAS nlohmann)

target_include_directories(nlohmann
    INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)