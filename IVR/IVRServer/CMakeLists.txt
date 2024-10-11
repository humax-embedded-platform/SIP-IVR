cmake_minimum_required(VERSION 3.16)

project(IVRServer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# add header dir
include_directories(.)

add_executable(IVRServer
  main.cpp
  SipMessage.h SipMessage.cpp
  SdpSipMessage.h SdpSipMessage.cpp
  UdpClient.h UdpClient.cpp
  CallDetails.h CallDetails.cpp
)

# target_link_libraries(IVRServer Qt${QT_VERSION_MAJOR}::Core)

include(GNUInstallDirs)
install(TARGETS IVRServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
