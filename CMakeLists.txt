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

IF(NOT DEFINED ENV{MSGPACKPATH})
  MESSAGE(FATAL_ERROR "Please set MSGPACKPATH to the directory where MessagePack is installed and execute cmake again.")
ENDIF(NOT DEFINED ENV{MSGPACKPATH})

STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
SET(MSGPACKPATH $ENV{MSGPACKPATH})

include_directories(${ZMQPATH}/include ${MSGPACKPATH}/include)

#Define the list of the source files
set(SOURCES zmqproto.cxx zmqproto.h ${ZMQPATH}/include/zmq.hpp ${MSGPACKPATH}/include/msgpack.hpp)
set(FLPSOURCES zmqprotoPublisher.cxx zmqprotoPublisher.h zmqprotoCommon.cxx zmqprotoCommon.h ${ZMQPATH}/include/zmq.hpp ${MSGPACKPATH}/include/msgpack.hpp)
set(EPNSOURCES zmqprotoSubscriber.cxx zmqprotoSubscriber.h zmqprotoCommon.cxx zmqprotoCommon.h ${ZMQPATH}/include/zmq.hpp)

#Seek for additional libraries here
link_directories(${ZMQPATH}/lib)
link_directories(${MSGPACKPATH}/lib)

#Compile the executables
add_executable(zmqprotodirectory ${SOURCES})
add_executable(zmqprotoflp ${FLPSOURCES})
add_executable(zmqprotoepn ${EPNSOURCES})

#Link the executables
target_link_libraries (zmqprotodirectory zmq msgpack)
target_link_libraries (zmqprotoflp zmq msgpack pthread)
target_link_libraries (zmqprotoepn zmq pthread)

#Install the executables
install(TARGETS zmqprotodirectory DESTINATION bin)
install(TARGETS zmqprotoflp DESTINATION bin)
install(TARGETS zmqprotoepn DESTINATION bin)
