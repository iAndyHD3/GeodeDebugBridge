cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)
project(GeodeDebugBridge VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})


CPMAddPackage("gh:stephenberry/glaze@7.0.2")


# Fetch websocketpp using CPM
CPMAddPackage(
    NAME websocketpp
    GITHUB_REPOSITORY zaphoyd/websocketpp
    GIT_TAG 4dfe1be
    DOWNLOAD_ONLY YES
)

# Fetch standalone asio for websocketpp
CPMAddPackage(NAME asio GITHUB_REPOSITORY chriskohlhoff/asio GIT_TAG asio-1-24-0 DOWNLOAD_ONLY YES)

# Create interface library for websocketpp
if(websocketpp_ADDED)
    add_library(websocketpp INTERFACE)
    target_include_directories(websocketpp INTERFACE 
        ${websocketpp_SOURCE_DIR}
        ${asio_SOURCE_DIR}/asio/include
    )
    target_compile_definitions(
        websocketpp
        INTERFACE
            ASIO_STANDALONE
            _WEBSOCKETPP_CPP11_STL_
            _WEBSOCKETPP_CPP11_FUNCTIONAL_
    )
endif()


target_link_libraries(${PROJECT_NAME} websocketpp glaze::glaze)