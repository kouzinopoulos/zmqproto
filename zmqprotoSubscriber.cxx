#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoSubscriber.h"

using namespace std;

char localIPAddr[30], directoryIPAddr[30];

void *pushToDirectory (void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  //Initialize socket
  zmq::socket_t directorySocket(*context, ZMQ_PUSH);
  directorySocket.connect(directoryIPAddr);
  
  //On each time slice, send the whole "tcp://IP:port" identifier to the directory
  while (1) {
    zmq::message_t msgToDirectory(30);
    memcpy(msgToDirectory.data(), localIPAddr, 30);
    directorySocket.send (msgToDirectory);
    
    cout << "EPN: Sent a ping to the directory" << endl;
    
    sleep(10);
  }
}

void *pullFromFLP (void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  //Initialize socket
  zmq::socket_t FLPSocket(*context, ZMQ_PULL);
  FLPSocket.bind(localIPAddr);
  
  //Receive payload from the FLPs
  while (1) {
    zmq::message_t msgFromFLP;
    FLPSocket.recv (&msgFromFLP);
    
    Content* input = reinterpret_cast<Content*>(msgFromFLP.data());
    
    cout << "EPN: Received payload " << (&input[0])->id << " from FLP" << endl;
    cout << "EPN: Message size: " << msgFromFLP.size() << " bytes" << endl;
    cout << "EPN: message content: " << (&input[0])->x << " " << (&input[0])->y << " " << (&input[0])->z << " " << (&input[0])->a << " " << (&input[0])->b << endl << endl;
  }
}

int main(int argc, char** argv)
{
  if ( argc != 5 ) {
    cout << "Usage: " << argv[0] << " localIPAddr localIPPort directoryIPAddr directoryIPPort" << endl;
    return 1;
  }
  
  snprintf(localIPAddr, 30, "tcp://%s:%s", argv[1], argv[2]);
  snprintf(directoryIPAddr, 30, "tcp://%s:%s", argv[3], argv[4]);
  
  zmq::context_t context (1);
  
  //Launch the threads that handle the sockets
  pthread_t directoryThread;
  pthread_create (&directoryThread, NULL, pushToDirectory, &context);
  
  pthread_t FLPThread;
  pthread_create (&FLPThread, NULL, pullFromFLP, &context);
  
  //Wait for the threads to exit
  (void) pthread_join(directoryThread, NULL);
  (void) pthread_join(FLPThread, NULL);
}
