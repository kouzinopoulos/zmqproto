#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoSubscriber.h"

using namespace std;

char directoryIPAddr[30];

void *pushToDirectory (void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  char *localIP = zmqprotoCommon::determine_ip();
  
  //Initialize socket
  zmq::socket_t directorySocket(*context, ZMQ_PUSH);
  directorySocket.connect(directoryIPAddr);
  
  //Send a ping per time slice to the directory
  while (1) {
    zmq::message_t msgToDirectory(20);
    memcpy(msgToDirectory.data(), localIP, 20);
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
  FLPSocket.bind("tcp://*:5560");
  
  //Receive payload from the FLPs
  while (1) {
    zmq::message_t msgFromFLP;
    FLPSocket.recv (&msgFromFLP);
    
    cout << "EPN: Received payload from FLP" << endl;
    cout << "EPN: Message size: " << msgFromFLP.size() << " bytes." << endl;
    
    Content* input = reinterpret_cast<Content*>(msgFromFLP.data());
    cout << "EPN: message content: " << (&input[0])->x << " " << (&input[0])->y << " " << (&input[0])->z << " " << (&input[0])->a << " " << (&input[0])->b << endl << endl;
  }
}

int main(int argc, char** argv)
{
  if ( argc != 2 ) {
    cout << "Usage: "<<argv[0]<<" directoryIPAddr" << endl;
    return 1;
  }
  
  snprintf(directoryIPAddr, 30, "tcp://%s:5558", argv[1]);
  
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
