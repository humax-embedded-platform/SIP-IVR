cmake_minimum_required(VERSION 3.16)

project(MediaServer LANGUAGES CXX)

#add a github repo as dependency
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
endif()

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include_directories(.)
include_directories(./include)

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GSTRTP REQUIRED gstreamer-rtp-1.0)
pkg_check_modules(GSTAUDIO REQUIRED gstreamer-audio-1.0)
pkg_check_modules(OPENSSL REQUIRED openssl)

link_directories(
    ${GLIB_LIBRARY_DIRS}
    ${GST_LIBRARY_DIRS}
    ${GSTRTP_LIBRARY_DIRS}
    ${GSTAUDIO_LIBRARY_DIRS}
    ${OPENSSL_LIBRARY_DIRS}
)

include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${GSTRTP_INCLUDE_DIRS}
    ${GSTAUDIO_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
)

set(LIBS
    ${GLIB_LDFLAGS}
    ${GST_LDFLAGS}
    ${GSTRTP_LDFLAGS}
    ${GSTAUDIO_LDFLAGS}
    ${OPENSSL_LDFLAGS}
    spdlog
    dl
)

add_executable(MediaServer
  main.cpp
  MediaServer.h MediaServer.cpp
  RequestHandler.h RequestHandler.cpp
  SessionManager.h SessionManager.cpp
  MediaSession.h MediaSession.cpp
  util/HashUtil.h util/HashUtil.cpp
  player/GstPlayer.h player/GstPlayer.cpp
  util/FileUtil.h util/FileUtil.cpp
  player/GstPlayerContext.h
  player/GstSenderPlayer.h player/GstSenderPlayer.cpp
  player/GstReceiverPlayer.h player/GstReceiverPlayer.cpp
  player/GstPlayerContext.cpp
  player/GstBasePlayer.h player/GstBasePlayer.cpp
  Defines.h

)

target_link_libraries(MediaServer ${LIBS})

include(GNUInstallDirs)
install(TARGETS MediaServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# set install dir
set_target_properties(MediaServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)
