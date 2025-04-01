cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(mp_os LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message([[VCPKG toolchain setup example. It is needed for some libraries. Highly recommended to pass it by command line arguments for cmake configuration (IDE can help with this).
        set(VCPKG_TARGET_TRIPLET x64-windows)
        set(CMAKE_TOOLCHAIN_FILE C:/Programming/vcpkg/scripts/buildsystems/vcpkg.cmake)]])

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

set(CROW_BUILD_EXAMPLES OFF)
set(CROW_BUILD_TESTS OFF)
set(CROW_INSTALL ON)

FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(crow)

find_package(Boost COMPONENTS system container REQUIRED) # Ставить через vcpkg

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

add_library(crow_lib INTERFACE)
target_include_directories(crow_lib INTERFACE ${CMAKE_BINARY_DIR}/_deps/crow-src/include)
target_link_libraries(crow_lib INTERFACE Boost::boost)
target_compile_definitions(crow_lib INTERFACE 
    BOOST_ASIO_ENABLE_OLD_SERVICES
    BOOST_ALLOW_DEPRECATED_HEADERS)

add_library(crow ALIAS crow_lib)

add_subdirectory(allocator)
add_subdirectory(arithmetic)
add_subdirectory(associative_container)
add_subdirectory(common)
add_subdirectory(logger)