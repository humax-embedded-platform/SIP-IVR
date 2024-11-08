cmake_minimum_required (VERSION 3.8)

project ("SipServer")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

set(HELPERS_SRC_FILES "${SRC_DIR}/Helpers")
file(GLOB_RECURSE HELPERS_SRC
    "${HELPERS_SRC_FILES}/*.cpp"
    "${HELPERS_SRC_FILES}/*.h"
    "${HELPERS_SRC_FILES}/*.hpp"
)

set(SIP_SRC_FILES "${SRC_DIR}/SIP")
file(GLOB_RECURSE SIP_SRC
    "${SIP_SRC_FILES}/*.cpp"
    "${SIP_SRC_FILES}/*.h"
    "${SIP_SRC_FILES}/*.hpp"
)

include_directories(${HELPERS_SRC_FILES} ${SIP_SRC_FILES})
add_executable (SipServer "main.cpp" ${SIP_SRC} ${HELPERS_SRC} )
target_link_libraries(SipServer -pthread spdlog dl)

source_group("Helpers"  FILES ${HELPERS_SRC_FILES})
source_group("SIP"      FILES ${SIP_SRC_FILES})

# set install dir
set_target_properties(SipServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)
