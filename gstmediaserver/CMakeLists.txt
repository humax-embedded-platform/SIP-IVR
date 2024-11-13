cmake_minimum_required(VERSION 3.16)

project(MediaServer LANGUAGES CXX)

set(LIB "gstmediaserver")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include_directories(.)
include_directories(./include)
include_directories(./src)
include_directories(./src/player)
include_directories(./src/util)
include_directories(${PRO_DIR}/include)

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

add_library(${LIB} SHARED
  src/MediaServer.cpp
  src/RequestHandler.cpp
  src/SessionManager.cpp
  src/MediaSession.cpp
  src/util/HashUtil.cpp
  src/player/GstPlayer.cpp
  src/util/FileUtil.cpp
  src/player/GstSenderPlayer.cpp
  src/player/GstReceiverPlayer.cpp
  src/player/GstPlayerContext.cpp
  src/player/GstBasePlayer.cpp
  include/gstmediaserver.h
  src/gstmediaserver.cpp
)

target_link_libraries(${LIB} PRIVATE ${LIBS})

target_include_directories(${LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(${LIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${BIN_DIR}"
)
