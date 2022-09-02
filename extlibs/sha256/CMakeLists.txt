project(sha256)

file(GLOB SHA256_HEADERS include/sha256/*.hpp)
file(GLOB SHA256_SOURCES src/*.cpp)

add_library(sha256 ${SHA256_HEADERS} ${SHA256_SOURCES})

target_include_directories(sha256
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)