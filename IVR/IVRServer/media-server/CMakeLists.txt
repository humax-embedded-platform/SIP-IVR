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


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
)

add_executable(MediaServer
  main.cpp
  MediaServer.h MediaServer.cpp
  RequestHandler.h RequestHandler.cpp
  SessionManager.h SessionManager.cpp
  MediaSession.h MediaSession.cpp
  util/HashUtil.h util/HashUtil.cpp
  player/GstPlayer.h player/GstPlayer.cpp
)

target_link_libraries(MediaServer ${LIBS})

include(GNUInstallDirs)
install(TARGETS MediaServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
