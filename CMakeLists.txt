cmake_minimum_required(VERSION 3.16)

project(sip-ivr LANGUAGES CXX)

set(APP "sip-ivr")
set(PRO_DIR ${CMAKE_SOURCE_DIR})
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
endif()

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)

# include sub project
add_subdirectory(ivr/gstivr)
add_subdirectory(sipserver)
add_subdirectory(gstmediaserver)

include_directories(./include)
include_directories(./common)

add_executable(${APP}
    src/main.cpp
    include/BaseLogger.hpp
)

target_link_libraries(${APP} sipserver gstmediaserver gstivr spdlog dl)

# install
set_target_properties(${APP} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)
