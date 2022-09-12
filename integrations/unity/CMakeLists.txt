cmake_minimum_required(VERSION 3.21)

project(neartoolbox_unity)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/neartoolbox)

file(GLOB NEAR_TB_UNITY_HEADERS include/ntb_unity/*.h)
file(GLOB NEAR_TB_UNITY_SOURCES src/*.cpp)

add_library(neartoolbox_unity SHARED ${NEAR_TB_UNITY_HEADERS} ${NEAR_TB_UNITY_SOURCES})

target_link_libraries(neartoolbox_unity neartoolbox)

target_include_directories(neartoolbox
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(neartoolbox PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
