cmake_minimum_required(VERSION 3.16)

project(gstivr LANGUAGES CXX)

set(LIB "gstivr")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

# include(FetchContent)
# FetchContent_Declare(
#     spdlog
#     GIT_REPOSITORY https://github.com/gabime/spdlog.git
#     GIT_TAG v1.14.1
# )
# FetchContent_MakeAvailable(spdlog)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# add header dir
include_directories(./src)
include_directories(./src/model)
include_directories(./src/media)
include_directories(./src/helpers)
include_directories(./src/include)
include_directories(${PRO_DIR}/include)

add_library(${LIB} SHARED
  # main.cpp
  src/Application.hpp src/Application.cpp
  src/model/SipMessage.hpp src/model/SipMessage.cpp
  src/model/SipSdpMessage.hpp src/model/SipSdpMessage.cpp
  src/UdpClient.hpp src/UdpClient.cpp
  src/CallDetails.h src/CallDetails.cpp
  src/SipClient.hpp src/SipClient.cpp
  src/helpers/cxxopts.hpp src/helpers/IDGen.hpp src/helpers/Log.hpp
  src/SipMessageTypes.h
  src/SipMessageHeaders.h
  src/session/SessionManager.h src/session/SessionManager.cpp
  src/session/CallSession.hpp src/session/CallSession.cpp
  src/media/MediaSession.h src/media/MediaSession.cpp
  src/media/MediaManager.h src/media/MediaManager.cpp
  src/SipMessageFactory.cpp src/SipMessageFactory.hpp
  src/AppDefines.h
  src/media/MediaClient.h src/media/MediaClient.cpp
  src/DTMFHandler.h src/DTMFHandler.cpp
  src/helpers/NetUtil.h src/helpers/NetUtil.cpp
  include/gstivr.h src/gstivr.cpp
)

target_link_libraries(${LIB} PRIVATE spdlog dl)

set_target_properties(${LIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${BIN_DIR}"
)

target_include_directories(${LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
