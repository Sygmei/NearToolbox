project(borsch)

file(GLOB BORSCH_HEADERS include/borsch/borsch.hpp)

add_library(borsch INTERFACE ${BORSCH_HEADERS})
add_library(borsch::borsch ALIAS borsch)

target_include_directories(borsch
    INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)