cmake_minimum_required(VERSION 2.8)

project(0mqproto)

#Set the version of the software
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_PATCH 3)

#ZeroMQ is part of the external packages of FairRoot.
IF(NOT DEFINED ENV{ZMQPATH})
  MESSAGE(FATAL_ERROR "Please set ZMQPATH to the directory where ZeroMQ is installed and execute cmake again.")
ENDIF(NOT DEFINED ENV{ZMQPATH})

STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
SET(ZMQPATH $ENV{ZMQPATH})

include_directories(${ZMQPATH}/include)

#find_package(ZeroMQ)

#Define the list of the source files
set(SOURCES 0mqproto.c zhelpers.h ${ZMQPATH}/include/zmq.h)

#Seek for additional libraries here
link_directories(${ZMQPATH}/lib)

#Compile the executable
add_executable(0mqproto ${SOURCES})

#Link the executable
target_link_libraries (0mqproto zmq pthread)

#append the custom linker flags to the corresponding cmake variables
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${PROJECT_LINK_FLAGS}")

install(TARGETS 0mqproto DESTINATION bin)
