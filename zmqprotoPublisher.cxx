#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <unistd.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "zmqprotoCommon.h"
#include "zmqprotoContext.h"
#include "zmqprotoSocket.h"
#include "zmqprotoPublisher.h"

using namespace std;

void Log (zmqprotoSocket *pullFromDirectoryPtr, zmqprotoSocket *pushToEPNPtr)
{
  unsigned long bytesTx = 0;
  unsigned long messagesTx = 0;
  unsigned long bytesRx = 0;
  unsigned long messagesRx = 0;
  
  while (1) {
    cout << "Tx " << "\033[01;34m" << pushToEPNPtr->GetBytesTx() - bytesTx << " b/s "
         << pushToEPNPtr->GetMessagesTx() - messagesTx << " msg/s \033[0m Rx " << "\033[01;31m"
         << pullFromDirectoryPtr->GetBytesRx() - bytesRx << " b/s "
         << pullFromDirectoryPtr->GetMessagesRx() - messagesRx << " msg/s" << "\033[0m" << endl;
         
    bytesTx = pushToEPNPtr->GetBytesTx();
    messagesTx = pushToEPNPtr->GetMessagesTx();
    bytesRx = pullFromDirectoryPtr->GetBytesRx();
    messagesRx = pullFromDirectoryPtr->GetMessagesRx();
    
    boost::this_thread::sleep(boost::posix_time::seconds(1));
  }
}

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
  int numIoThreads = 1;
  zmqprotoContext fContext (numIoThreads);
  
  //Initialize socket to pull from the directory
  zmqprotoSocket pullFromDirectory(fContext.GetContext(), "pull", 0);
  pullFromDirectory.Connect(directoryIPAddr);
  
  //Initialize socket to push to EPNs
  zmqprotoSocket pushToEPN(fContext.GetContext(), "push", 1);
  
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
  
  boost::thread logThread(Log, &pullFromDirectory, &pushToEPN);
  
  while (1) {
    //Receive the IP vector from the Directory
    zmq_msg_t msgFromDirectory;
    zmq_msg_init (&msgFromDirectory);
    int retVal = pullFromDirectory.Receive(&msgFromDirectory, "no-block");
    
    //If a message was received, unpack it
    if (retVal) {
    
      // Deserialize received message
      msgpack::unpacked unpacked;
      msgpack::unpack(&unpacked, reinterpret_cast<char *>(zmq_msg_data (&msgFromDirectory)), zmq_msg_size(&msgFromDirectory));
      
      msgpack::object obj = unpacked.get();
      obj.convert(&data);
    
      //cout << "FLP: Received a vector with " << data.size() << " IPs from the directory node" << endl;
      
      //Connect to all nodes in the vector
      for (int i = 0; i < data.size(); i++) {
        pushToEPN.Connect(data.at(i).c_str());
      }
    }
      
    //If there are connected EPNs, push data to them
    if ( data.size() > 0 ) {
      zmq_msg_t msgToEPN;
      zmq_msg_init_size (&msgToEPN, fEventSize * sizeof(Content));
      
      memcpy (zmq_msg_data(&msgToEPN), payload, fEventSize * sizeof(Content));
      pushToEPN.Send(&msgToEPN, "");
      /*
      cout << "FLP: Sent message " << (&payload[0])->id << " to EPN" << endl;
      cout << "FLP: Message size: " << fEventSize * sizeof(Content) << " bytes." << endl;
      cout << "FLP: Message content: " <<  (&payload[0])->id << " " << (&payload[0])->x << " " 
            << (&payload[0])->y << " " << (&payload[0])->z << " " << (&payload[0])->a << " " 
            << (&payload[0])->b << endl << endl;
      */
      //Increase the event ID
      (&payload[0])->id++;
      
      sleep(1);
    }
  }
  
  logThread.join();
  
  return 0;
}
