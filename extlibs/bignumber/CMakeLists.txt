project(bignumber)

file(GLOB BIGNUMBER_HEADERS include/bignumber/*.hpp)
file(GLOB BIGNUMBER_SOURCES src/*.cpp)

add_library(bignumber ${BIGNUMBER_HEADERS} ${BIGNUMBER_SOURCES})

target_include_directories(bignumber
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)