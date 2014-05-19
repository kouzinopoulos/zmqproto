#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoSubscriber.h"

using namespace std;

int main(int argc, char** argv)
{
  if ( argc != 2 ) {
    cout << "Usage: "<<argv[0]<<" directoryIPAddr" << endl;
    return 1;
  }
  
  char directoryIPAddr[30];
  
  snprintf(directoryIPAddr, 30, "tcp://%s:5558", argv[1]);
  
  zmq::context_t context (1);
  
  int fEventSize = 10000;
  char *localIP = zmqprotoCommon::determine_ip();
  
  //Initialize subscriber socket
  zmq::socket_t pushToDirectory(context, ZMQ_PUSH);
  pushToDirectory.connect(directoryIPAddr);
  
  //Initialize socket to receive from FLP
  zmq::socket_t pullFromFLP(context, ZMQ_PULL);
  pullFromFLP.bind("tcp://*:5560");
  
  while (1) {
    //Send a ping to the directory, 1 per second
    zmq::message_t msgToDirectory(20);
    memcpy(msgToDirectory.data(), localIP, 20);
    pushToDirectory.send (msgToDirectory);
    
    cout << "EPN: Sent a ping to the directory" << endl;
    
    //Receive payload from the FLPs
    zmq::message_t msgFromFLP;
    pullFromFLP.recv (&msgFromFLP);
    
    cout << "EPN: Received payload from FLP" << endl;
    cout << "EPN: Message size: " << msgFromFLP.size() << " bytes." << endl;
    
    Content* input = reinterpret_cast<Content*>(msgFromFLP.data());
    cout << "EPN: message content: " << (&input[0])->x << " " << (&input[0])->y << " " << (&input[0])->z << " " << (&input[0])->a << " " << (&input[0])->b << endl;
    
    sleep(1);
  }
}
