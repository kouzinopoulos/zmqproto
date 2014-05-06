cmake_minimum_required(VERSION 2.8)

project(zmqproto)

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

#Define the list of source files
set(BROKERSOURCES broker.c zhelpers.h ${ZMQPATH}/include/zmq.h)
set(FLPCLIENTSOURCES flpclient.c zhelpers.h ${ZMQPATH}/include/zmq.h)
set(EPNCLIENTSOURCES epnclient.c zhelpers.h ${ZMQPATH}/include/zmq.h)

#Seek for additional libraries here
link_directories(${ZMQPATH}/lib)

#Compile the executables
add_executable(broker ${BROKERSOURCES})
add_executable(flpclient ${FLPCLIENTSOURCES})
add_executable(epnclient ${EPNCLIENTSOURCES})

#Link the executables
target_link_libraries (broker zmq pthread)
target_link_libraries (flpclient zmq pthread)
target_link_libraries (epnclient zmq pthread)

#Install the executables
install(TARGETS broker DESTINATION bin)
install(TARGETS flpclient DESTINATION bin)
install(TARGETS epnclient DESTINATION bin)
