cmake_minimum_required(VERSION 3.16)

project(Sip-IVR LANGUAGES CXX)

# include sub project
add_subdirectory(IVR/IVRGST)
add_subdirectory(SIPSever)
add_subdirectory(GstMediaServer)
