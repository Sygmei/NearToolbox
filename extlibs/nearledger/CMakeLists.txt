cmake_minimum_required(VERSION 3.16)
project(NearLedger C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(hidapi_name)

if(WIN32)
elseif(APPLE)
    set(hidapi_name hidapi)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(PLATFORM_LINUX)
    set(hidapi_name hidapi-libusb)
endif()

set(NEARLEDGER_SOURCE
    "include/nearledger/comm.hpp"
    "include/nearledger/error.hpp"
    "include/nearledger/hid_device.hpp"
    "include/nearledger/ledger.hpp"
    "include/nearledger/transport.hpp"
    "include/nearledger/utils.hpp"
    "src/error.cpp"
    "src/hid_device.cpp"
    "src/ledger.cpp"
    "src/transport.cpp"
    "src/utils.cpp"
)

add_library(nearledger ${NEARLEDGER_SOURCE})
add_library(nearledger::nearledger ALIAS nearledger)

add_subdirectory(extlibs/hidapi)
target_link_libraries(nearledger hidapi::hidapi)

target_include_directories(nearledger
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src
)

file(GLOB_RECURSE EDV_INCLUDE_FILES "include/*.hpp*")
set_target_properties(nearledger
    PROPERTIES
    PUBLIC_HEADER "${EDV_INCLUDE_FILES}"
)

include(GNUInstallDirs)
install(TARGETS nearledger
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/include
)

set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples ?")

if(BUILD_EXAMPLES)
    add_executable(nearledger_examples examples/main.cpp)
    target_link_libraries(nearledger_examples nearledger::nearledger)
endif()