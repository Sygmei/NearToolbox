project(borsh)

file(GLOB BORSH_HEADERS include/borsh/*.hpp)

add_library(borsh INTERFACE ${BORSH_HEADERS})
add_library(borsh::borsh ALIAS borsh)

target_include_directories(borsh
    INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)