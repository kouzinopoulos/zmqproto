#include <zmq.hpp>

#include <iostream>

#include "zmqprotoCommon.h"
#include "zmqprotoSubscriber.h"

using namespace std;

void *zmqprotoSubscriber::Run(void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  pthread_t self_id = pthread_self();
  
  int counter = 0;
  int fEventSize = 10000;
  
  //Initialize subscriber socket
  zmq::socket_t subscriber(*context, ZMQ_PULL);
  subscriber.connect("tcp://localhost:5558");
  
  //Receive messages from the subscriber
  while (1) {
    zmq::message_t msg (fEventSize * sizeof(Content));
    subscriber.recv (&msg); // First, discard the DEALER header (see http://stackoverflow.com/questions/16692807/)
    subscriber.recv (&msg);
    
    Content* input = reinterpret_cast<Content*>(msg.data());

    cout << self_id << " " << determine_ip() << " <- Recv message " << counter++ << " " << (&input[0])->x << " " << (&input[0])->y << " " << (&input[0])->z << " " << (&input[0])->a << " " << (&input[0])->b << endl;
  }
}
