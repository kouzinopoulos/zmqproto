cmake_minimum_required(VERSION 2.6)

project(zmqproto)

#Set the version of the software
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 0)

#Turn off/on debug messages
add_definitions( -DDEBUGMSG )

#ZeroMQ is part of the external packages of FairRoot
IF(NOT DEFINED ENV{ZMQPATH})
  MESSAGE(FATAL_ERROR "Please set ZMQPATH to the directory where ZeroMQ is installed and execute cmake again.")
ENDIF(NOT DEFINED ENV{ZMQPATH})

STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
SET(ZMQPATH $ENV{ZMQPATH})

#MsgPack must be installed separately
IF(NOT DEFINED ENV{MSGPACKPATH})
  MESSAGE(FATAL_ERROR "Please set MSGPACKPATH to the directory where MessagePack is installed and execute cmake again.")
ENDIF(NOT DEFINED ENV{MSGPACKPATH})

STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
SET(MSGPACKPATH $ENV{MSGPACKPATH})

include_directories(${ZMQPATH}/include ${MSGPACKPATH}/include)

#Define the list of the source files
set(SOURCES zmqprotoDirectory.cxx zmqprotoDirectory.h zmqprotoCommon.cxx zmqprotoCommon.h zmqprotoMessage.cxx zmqprotoMessage.h zmqprotoContext.cxx zmqprotoContext.h zmqprotoSocket.cxx zmqprotoSocket.h ${ZMQPATH}/include/zmq.hpp ${MSGPACKPATH}/include/msgpack.hpp)
set(FLPSOURCES zmqprotoFLP.cxx zmqprotoFLP.h zmqprotoCommon.cxx zmqprotoCommon.h zmqprotoMessage.cxx zmqprotoMessage.h zmqprotoContext.cxx zmqprotoContext.h zmqprotoSocket.cxx zmqprotoSocket.h ${ZMQPATH}/include/zmq.hpp ${MSGPACKPATH}/include/msgpack.hpp)
set(EPNSOURCES zmqprotoEPN.cxx zmqprotoEPN.h zmqprotoCommon.cxx zmqprotoCommon.h zmqprotoMessage.cxx zmqprotoMessage.h zmqprotoContext.cxx zmqprotoContext.h zmqprotoSocket.cxx zmqprotoSocket.h ${ZMQPATH}/include/zmq.hpp)

#Seek for additional libraries here
link_directories(${ZMQPATH}/lib)
link_directories(${MSGPACKPATH}/lib)

#Compile the executables
add_executable(zmqprotodirectory ${SOURCES})
add_executable(zmqprotoflp ${FLPSOURCES})
add_executable(zmqprotoepn ${EPNSOURCES})

#Link the executables
target_link_libraries (zmqprotodirectory zmq msgpack boost_system boost_thread)
target_link_libraries (zmqprotoflp zmq msgpack boost_system boost_thread)
target_link_libraries (zmqprotoepn zmq boost_system boost_thread)

#Install the executables
install(TARGETS zmqprotodirectory DESTINATION bin)
install(TARGETS zmqprotoflp DESTINATION bin)
install(TARGETS zmqprotoepn DESTINATION bin)
