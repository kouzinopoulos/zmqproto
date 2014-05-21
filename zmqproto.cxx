#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <vector>

#include <unistd.h>

#include <sys/ioctl.h> //For ioctl
#include <net/if.h>
#include <netinet/in.h>

#include "zmqproto.h"

int main(int argc, char** argv)
{
  //Initialize zmq
  zmq::context_t context (1);
  
  //Initialize the IP vector
  vector<string>ipVector;
  vector<string>::iterator ipVectorIter;
  
  //Setup the directory sockets
  zmq::socket_t frontend (context, ZMQ_PUSH);
  frontend.bind("tcp://*:5556");

  zmq::socket_t backend (context, ZMQ_PULL);
  backend.bind("tcp://*:5558");
  
  cout << "Directory: Waiting for incoming connections..." << endl;
  
  while (1) {
    zmq::message_t subMsg (20 * sizeof(char));
    backend.recv(&subMsg);
    
    //cout << reinterpret_cast<char *>(subMsg.data()) <<endl;
    cout << "Directory: Received a ping from EPN" << endl;
    
    //If the IP is unknown, add it to the IP vector
    ipVectorIter = find (ipVector.begin(), ipVector.end(), reinterpret_cast<char *>(subMsg.data()));
    
    if ( ipVectorIter == ipVector.end() ) {
      //FIXME: performance wise is better to allocate big blocks of memory for the vector instead of element-by-element with each push_back()
      ipVector.push_back(reinterpret_cast<char *>(subMsg.data()));
      
      cout << "Directory: Unknown IP, adding it to vector. Total IPs: " << ipVector.size() << endl;
    }
    
    //Pack the IP vector using msgpack and send it to all connected FLPs
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, ipVector);

    zmq::message_t pubMsg(sbuf.size());
    memcpy(pubMsg.data(), sbuf.data(), sbuf.size());

    frontend.send(pubMsg);
    
    cout << "Directory: Sent the IP vector to all FLPs" << endl << endl;
  }
  return 0;
}

