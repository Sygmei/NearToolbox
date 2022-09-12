project(neartoolbox_unity)

file(GLOB NEAR_TB_UNITY_HEADERS include/ntb_unity/*.h)
file(GLOB NEAR_TB_UNITY_SOURCES src/*.cpp)

add_library(neartoolbox_unity SHARED ${NEAR_TB_UNITY_HEADERS} ${NEAR_TB_UNITY_SOURCES})

target_link_libraries(neartoolbox_unity neartoolbox)

target_include_directories(neartoolbox_unity
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
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
