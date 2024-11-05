cmake_minimum_required(VERSION 3.16)

project(Sip-IVR LANGUAGES CXX)

set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
message(STATUS "Binary directory: ${BIN_DIR}")

# include sub project
add_subdirectory(IVR/IVRGST)
add_subdirectory(SIPSever)
add_subdirectory(GstMediaServer)
