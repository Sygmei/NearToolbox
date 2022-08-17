cmake_minimum_required(VERSION 3.10)

project(neartoolbox)

add_subdirectory(extlibs/ed25519)

file(GLOB NEAR_TB_HEADERS include/*.hpp)
file(GLOB NEAR_TB_SOURCES src/*.cpp)

add_library(neartoolbox SHARED ${NEAR_TB_HEADERS} ${NEAR_TB_SOURCES})

target_link_libraries(neartoolbox ed25519)

target_include_directories(neartoolbox
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)