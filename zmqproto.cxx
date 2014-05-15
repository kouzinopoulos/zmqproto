#include <zmq.hpp>

#include <iostream>
#include <vector>

#include <unistd.h>

#include <sys/ioctl.h> //For ioctl
#include <net/if.h>
#include <netinet/in.h>

#include "zmqproto.h"
#include "zmqprotoPublisher.h"
#include "zmqprotoProxy.h"
#include "zmqprotoSubscriber.h"

int main(int argc, char** argv)
{
  if ( argc != 3 ) {
    cout << "Usage: zmqproto\tnumFLP\tnumEPN" << endl;
    return 1;
  }
  
  int numFLP = atoi(argv[1]);
  int numEPN = atoi(argv[2]);
  
  if ( numFLP <= 0 || numEPN <= 0 ) {
    cout << "Error: At least 1 FLP and EPN needed\n" << endl;
    return -1;
  }
  
  //Initialize zmq
  zmq::context_t context (1);
    
  //Start publisher on a new thread
  for ( int i = 0; i < numFLP; i++ ) {
    pthread_t publisherThread;
    pthread_create (&publisherThread, NULL, zmqprotoPublisher::Run, &context);
  }
  
  //Start subscriber on a new thread
  for ( int i = 0; i < numEPN; i++ ) {
    pthread_t subscriberThread;
    pthread_create (&subscriberThread, NULL, zmqprotoSubscriber::Run, &context);
  }
  
  //Block on proxy
  zmq::socket_t frontend (context, ZMQ_ROUTER);
  frontend.bind("tcp://*:5556");

  zmq::socket_t backend (context, ZMQ_DEALER);
  backend.bind("tcp://*:5558");

  //  Initialize poll set
  zmq::pollitem_t items [] = {
    { frontend, 0, ZMQ_POLLIN, 0 },
    { backend,  0, ZMQ_POLLIN, 0 }
  };
  
  vector<string>ipVector;
  vector<string>::iterator ipVectorIter;

  //  Switch messages between sockets
  while (1) {
    cout<<"Vector size: "<<ipVector.size()<<endl;
    
    for ( long vectorIndex = 0; vectorIndex < (long)ipVector.size(); vectorIndex++ ) {
      cout <<vectorIndex<<" - "<<ipVector.at(vectorIndex)<<endl;
    }

    zmq::message_t message;
    int64_t more;           //  Multipart detection

    zmq::poll (&items [0], 2, -1);

    if (items [0].revents & ZMQ_POLLIN) {
      while (1) {
        //  Process all parts of the message
        frontend.recv(&message);
        
        size_t more_size = sizeof (more);
        frontend.getsockopt(ZMQ_RCVMORE, &more, &more_size);
        backend.send(message, more? ZMQ_SNDMORE: 0);

        if (!more)
          break; //  Last message part
      }
    }
    if (items [1].revents & ZMQ_POLLIN) {
      while (1) {
        //  Process all parts of the message
        backend.recv(&message);
        
        size_t more_size = sizeof (more);
        backend.getsockopt(ZMQ_RCVMORE, &more, &more_size);
        frontend.send(message, more? ZMQ_SNDMORE: 0);
        
        if (!more)
          break; //  Last message part
      }
    }
    
    //Add the IP to the IP vector but only if it is not already in
    ipVectorIter = find (ipVector.begin(), ipVector.end(), zmqprotoCommon::determine_ip());
    
    if ( ipVectorIter == ipVector.end() ) {
      //FIXME: performance wise is better to allocate big blocks of memory for the vector instead of element-by-element with each push_back()
      ipVector.push_back(zmqprotoCommon::determine_ip());
    }
  }
 
  return 0;
}

