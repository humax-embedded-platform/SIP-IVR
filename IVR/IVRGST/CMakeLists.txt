cmake_minimum_required(VERSION 3.16)

project(IVRServer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# add header dir
include_directories(.)
include_directories(./model)
include_directories(./media)
include_directories(./helpers)
include_directories(./include)

add_executable(IVRServer
  main.cpp
  Application.hpp Application.cpp
  model/SipMessage.hpp model/SipMessage.cpp
  model/SipSdpMessage.hpp model/SipSdpMessage.cpp
  UdpClient.hpp UdpClient.cpp
  CallDetails.h CallDetails.cpp
  SipClient.hpp SipClient.cpp

  helpers/cxxopts.hpp helpers/IDGen.hpp helpers/Log.hpp
  SipMessageTypes.h
  SipMessageHeaders.h
  session/SessionManager.h session/SessionManager.cpp
  session/CallSession.hpp session/CallSession.cpp
  media/MediaSession.h media/MediaSession.cpp
  media/MediaManager.h media/MediaManager.cpp
  SipMessageFactory.cpp SipMessageFactory.hpp
  AppDefines.h
  media/MediaClient.h media/MediaClient.cpp
  DTMFHandler.h DTMFHandler.cpp
  helpers/NetUtil.h helpers/NetUtil.cpp
)

target_link_libraries(IVRServer spdlog)

include(GNUInstallDirs)
install(TARGETS IVRServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# set install dir
set_target_properties(IVRServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)
