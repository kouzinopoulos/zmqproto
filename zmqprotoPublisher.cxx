#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoPublisher.h"

using namespace std;

int main(int argc, char** argv)
{
  if ( argc != 2 ) {
    cout << "Usage: "<<argv[0]<<" directoryIPAddr" << endl;
    return 1;
  }
  
  char EPNIPAddr[30], directoryIPAddr[30];
  
  snprintf(directoryIPAddr, 30, "tcp://%s:5556", argv[1]);
    
  //Initialize zmq
  zmq::context_t context (1);
  
  //Initialize socket to pull from the directory
  zmq::socket_t pullFromDirectory(context, ZMQ_PULL);
  pullFromDirectory.connect(directoryIPAddr);
  
  //Initialize socket to push to EPNs
  zmq::socket_t pushToEPN(context, ZMQ_PUSH);
  
  //Initialize dummy data
  int fEventSize = 100000;
  
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
    for (int i = 0; i < data.size(); i++) {
      snprintf(EPNIPAddr, 30, "tcp://%s:5560", data.at(i).c_str());
      pushToEPN.connect(EPNIPAddr);
      
      zmq::message_t msgToEPN (fEventSize * sizeof(Content));
      memcpy(msgToEPN.data(), payload, fEventSize * sizeof(Content));
      pushToEPN.send (msgToEPN);
      
      cout << "FLP: Sent a message to EPN at " << data.at(i).c_str() << endl;
      cout << "FLP: Message size: " << fEventSize * sizeof(Content) << " bytes." << endl;
      cout << "FLP: Message content: " <<  (&payload[i])->id << " " << (&payload[i])->x << " " << (&payload[i])->y << " " << (&payload[i])->z << " " << (&payload[i])->a << " " << (&payload[i])->b << endl << endl;
      
      sleep(1);
    }
  }
  return 0;
}
