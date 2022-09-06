cmake_minimum_required(VERSION 3.21)

project(neartoolbox)

add_subdirectory(extlibs/base58)
add_subdirectory(extlibs/bignumber)
add_subdirectory(extlibs/borsh)
add_subdirectory(extlibs/ed25519)
add_subdirectory(extlibs/fmt)
add_subdirectory(extlibs/httplib)
add_subdirectory(extlibs/magic_enum)
add_subdirectory(extlibs/nearledger)
add_subdirectory(extlibs/nlohmann)
add_subdirectory(extlibs/result)
add_subdirectory(extlibs/sha256)

file(GLOB NEAR_TB_HEADERS include/ntb/*.hpp include/ntb/*.h)
file(GLOB NEAR_TB_SOURCES src/*.cpp)

# add_library(neartoolbox SHARED ${NEAR_TB_HEADERS} ${NEAR_TB_SOURCES})
add_executable(neartoolbox ${NEAR_TB_HEADERS} ${NEAR_TB_SOURCES})

target_link_libraries(neartoolbox base58)
target_link_libraries(neartoolbox bignumber)
target_link_libraries(neartoolbox borsh)
target_link_libraries(neartoolbox ed25519)
target_link_libraries(neartoolbox fmt)
target_link_libraries(neartoolbox httplib)
target_link_libraries(neartoolbox magic_enum)
target_link_libraries(neartoolbox nearledger)
target_link_libraries(neartoolbox nlohmann::json)
target_link_libraries(neartoolbox result)
target_link_libraries(neartoolbox sha256)

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
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)