cmake_minimum_required(VERSION 2.8)

project(zmqproto)

#Set the version of the software
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_PATCH 2)

#ZeroMQ is part of the external packages of FairRoot.
IF(NOT DEFINED ENV{ZMQPATH})
  MESSAGE(FATAL_ERROR "Please set ZMQPATH to the directory where ZeroMQ is installed and execute cmake again.")
ENDIF(NOT DEFINED ENV{ZMQPATH})

STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
SET(ZMQPATH $ENV{ZMQPATH})

include_directories(${ZMQPATH}/include)

#find_package(ZeroMQ)

#Define the list of the source files
set(SOURCES zmqproto.cxx zmqproto.h zmqprotoCommon.cxx zmqprotoCommon.h zmqprotoPublisher.cxx zmqprotoPublisher.h zmqprotoSubscriber.cxx zmqprotoSubscriber.h ${ZMQPATH}/include/zmq.hpp)

#Seek for additional libraries here
link_directories(${ZMQPATH}/lib)

#Compile the executable
add_executable(zmqproto ${SOURCES})

#Link the executable
target_link_libraries (zmqproto zmq pthread)

#append the custom linker flags to the corresponding cmake variables
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${PROJECT_LINK_FLAGS}")

install(TARGETS zmqproto DESTINATION bin)
