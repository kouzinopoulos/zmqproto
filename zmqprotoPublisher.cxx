#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoPublisher.h"

using namespace std;

int main(int argc, char** argv)
{
  if ( argc != 4 ) {
    cout << "Usage: " << argv[0] << " directoryIPAddr directoryIPPort fEventSize" << endl;
    return 1;
  }
  
  char directoryIPAddr[30];
  snprintf(directoryIPAddr, 30, "tcp://%s:%s", argv[1], argv[2]);
  
  int fEventSize = atoi(argv[3]);
  
  //Initialize zmq
  zmq::context_t context (1);
  
  //Initialize socket to pull from the directory
  zmq::socket_t pullFromDirectory(context, ZMQ_PULL);
  pullFromDirectory.connect(directoryIPAddr);
  
  //Initialize socket to push to EPNs
  zmq::socket_t pushToEPN(context, ZMQ_PUSH);
  
  Content* payload = new Content[fEventSize];
  for (int i = 0; i < fEventSize; ++i) {
    (&payload[i])->id = 0;
    (&payload[i])->x = rand() % 100 + 1;
    (&payload[i])->y = rand() % 100 + 1;
    (&payload[i])->z = rand() % 100 + 1;
    (&payload[i])->a = (rand() % 100 + 1) / (rand() % 100 + 1);
    (&payload[i])->b = (rand() % 100 + 1) / (rand() % 100 + 1);
  }

  std::vector<string> data;
  
  while (1) {
    //Receive the IP vector from the Directory
    zmq::message_t msgFromDirectory;
    int retVal = pullFromDirectory.recv(&msgFromDirectory, ZMQ_NOBLOCK);
    
    //If a message was received, unpack it
    if (retVal) {
    
      // Deserialize received message
      msgpack::unpacked unpacked;
      msgpack::unpack(&unpacked, reinterpret_cast<char*>(msgFromDirectory.data()), msgFromDirectory.size());
      
      msgpack::object obj = unpacked.get();
      obj.convert(&data);
    
      cout << "FLP: Received a vector with " << data.size() << " IPs from the directory node" << endl;
    }
        
    //Connect to each received IP, send payload
    //FIXME: connect only once to the EPNs, not on every data transfer?
    for (int i = 0; i < data.size(); i++) {
      pushToEPN.connect(data.at(i).c_str());
      
      zmq::message_t msgToEPN (fEventSize * sizeof(Content));
      memcpy(msgToEPN.data(), payload, fEventSize * sizeof(Content));
      pushToEPN.send (msgToEPN);
      
      cout << "FLP: Sent message " << (&payload[i])->id << " to EPN at " << data.at(i).c_str() << endl;
      cout << "FLP: Message size: " << fEventSize * sizeof(Content) << " bytes." << endl;
      cout << "FLP: Message content: " <<  (&payload[i])->id << " " << (&payload[i])->x << " " << (&payload[i])->y << " " << (&payload[i])->z << " " << (&payload[i])->a << " " << (&payload[i])->b << endl << endl;
      
      //Increase the event ID
      (&payload[i])->id++;
      
      sleep(1);
    }
  }
  return 0;
}
