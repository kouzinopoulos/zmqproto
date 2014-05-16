#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoSubscriber.h"

using namespace std;

void *zmqprotoSubscriber::Run(void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  pthread_t self_id = pthread_self();
  
  int fEventSize = 10000;
  
  //Initialize subscriber socket
  zmq::socket_t pushToDirectory(*context, ZMQ_PUSH);
  pushToDirectory.connect("tcp://127.0.0.1:5558");
  
  //Initialize socket to receive from FLP
  zmq::socket_t pullFromFLP(*context, ZMQ_PULL);
  
  pullFromFLP.bind("tcp://*:5560");
  
  while (1) {
    //Send a ping to the directory, 1 per second
    zmq::message_t msg(20);
    memcpy(msg.data(), determine_ip(), 20);
    pushToDirectory.send (msg);
    
    //Receive payload from the FLPs
    zmq::message_t msgFromFLP(fEventSize * sizeof(Content));
    pullFromFLP.recv (&msgFromFLP);
    
    cout << "EPN: received payload from FLP" << endl;
    
    sleep(1);
  }
}
