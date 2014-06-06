#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>

#include <time.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "zmqprotoCommon.h"
#include "zmqprotoContext.h"
#include "zmqprotoFLP.h"
#include "zmqprotoSocket.h"

using namespace std;

char directoryIPAddr[30];

std::vector<string> ipVector;
std::vector<string> ipVectorRecv;

bool updatedVector = false;
int connectedEPNs = 0;

void Log (zmqprotoSocket *pullFromDirectoryPtr, zmqprotoSocket *pushToEPNPtr)
{
  unsigned long bytesTx = 0;
  unsigned long messagesTx = 0;
  unsigned long bytesRx = 0;
  unsigned long messagesRx = 0;
  
  time_t start, current;
  time (&start);
  
  while (1) {
    time (&current);
    double difTime = difftime (current, start);
    
    cout << setprecision(0) << fixed
         << "EPNs: " << connectedEPNs << "/" << ipVector.size() <<  " "
         << "Tx \033[01;34m" << (pushToEPNPtr->GetBytesTx() - bytesTx)/1048576 << " Mb/s "
         << pushToEPNPtr->GetMessagesTx() - messagesTx << " msg/s \033[0m " << "Rx \033[01;31m"
         << (pullFromDirectoryPtr->GetBytesRx() - bytesRx)/1048576 << " Mb/s "
         << pullFromDirectoryPtr->GetMessagesRx() - messagesRx << " msg/s \033[0m "
         << "Avg Tx \033[01;34m" << pushToEPNPtr->GetBytesTx()/(difTime * 1048576) << " Mb/s "
         << pushToEPNPtr->GetMessagesTx()/difTime << " msg/s \033[0m Rx "
         << "\033[01;31m" << pullFromDirectoryPtr->GetBytesRx()/(difTime * 1048576) << " Mb/s "
         << pullFromDirectoryPtr->GetMessagesRx()/difTime << " msg/s \033[0m " << '\r' << flush;
         
    bytesTx = pushToEPNPtr->GetBytesTx();
    messagesTx = pushToEPNPtr->GetMessagesTx();
    bytesRx = pullFromDirectoryPtr->GetBytesRx();
    messagesRx = pullFromDirectoryPtr->GetMessagesRx();
    
    boost::this_thread::sleep (boost::posix_time::seconds(1));
  }
}

static bool compareVectors (vector<string> a, vector<string> b)
{
  if ( a.size() != b.size() ) {
   return false;
  }
  
  sort (a.begin(), a.end());
  sort (b.begin(), b.end());
  return (a == b);
}

void subscribeToDirectory (zmqprotoSocket *fSocketPtr)
{
  //Receive the IP vector from the Directory
  while (1) {
    //Connect to directory each time to get the updated vector
    fSocketPtr->Connect (directoryIPAddr);
    
    zmq_msg_t msg;
    zmq_msg_init (&msg);
    int retVal = fSocketPtr->Receive (&msg, "");
    
    //If a message was received, unpack it
    if (retVal) {
      // Deserialize received message
      msgpack::unpacked unpacked;
      msgpack::unpack (&unpacked, reinterpret_cast<char *>(zmq_msg_data (&msg)), zmq_msg_size (&msg));
      
      msgpack::object obj = unpacked.get();
      obj.convert (&ipVectorRecv);
      
      //If the received vector is different than the local, substitute the local with the received
      if (compareVectors (ipVector, ipVectorRecv) == false) {
        obj.convert (&ipVector);
        //FIXME: mutex lock it with boost to prevent race conditions
        updatedVector = true;
      }
    
      PRINT << "FLP: Received a vector with " << ipVector.size() << " IPs from the directory node";
    }
    zmq_msg_close (&msg);
  
    boost::this_thread::sleep (boost::posix_time::seconds(5));
  }
}

void pushToEPN (zmqprotoSocket *fSocketPtr, int fEventSize)
{
  //Allocate and initialize the payload
  Content* payload = new Content[fEventSize];
  for (int i = 0; i < fEventSize; ++i) {
    (&payload[i])->id = 0;
    (&payload[i])->x = rand() % 100 + 1;
    (&payload[i])->y = rand() % 100 + 1;
    (&payload[i])->z = rand() % 100 + 1;
    (&payload[i])->a = (rand() % 100 + 1) / (rand() % 100 + 1);
    (&payload[i])->b = (rand() % 100 + 1) / (rand() % 100 + 1);
  }
  
  while (1) {
   //If there are connected EPNs, push data to them
    if ( ipVector.size() > 0 ) {
      
      //Only (re)connect to the EPN nodes when the ip vector was updated
      if (updatedVector) {
        for (int i = 0; i < ipVector.size(); i++) {
          fSocketPtr->Connect (ipVector.at(i).c_str());
        }
        //FIXME: mutex lock it with boost to prevent race conditions
        updatedVector = false;
        
        connectedEPNs = ipVector.size();
      }
      
      zmq_msg_t msg;
      zmq_msg_init_size (&msg, fEventSize * sizeof(Content));
      
      memcpy (zmq_msg_data (&msg), payload, fEventSize * sizeof(Content));
      fSocketPtr->Send (&msg, "");
      
      PRINT << "Sent message " << (&payload[0])->id << " to EPN. Message size: "
            << fEventSize * sizeof(Content) << " bytes.";
      
      //Increase the event ID
      (&payload[0])->id++;
      
      zmq_msg_close (&msg);
    }
  }
}

int main(int argc, char** argv)
{
  if (argc != 4) {
    cout << "Usage: " << argv[0] << " directoryIPAddr directoryIPPort fEventSize" << endl;
    return 1;
  }
  
  if (atoi (argv[2]) > 65535 || atoi (argv[2]) < 1) {
    cout << "Usage: " << argv[0] << " directoryIPAddr directoryIPPort fEventSize" << endl;
    return 1;
  }
  
  snprintf (directoryIPAddr, 30, "tcp://%s:%s", argv[1], argv[2]);
  
  int fEventSize = atoi (argv[3]);
  
  //Initialize zmq
  int numIoThreads = 1;
  zmqprotoContext fContext (numIoThreads);
  
  //Initialize sockets
  zmqprotoSocket directorySocket (fContext.GetContext(), "sub", 0);
  zmqprotoSocket EPNSocket (fContext.GetContext(), "push", 1);
  
  //Launch the threads that handle the sockets
  boost::thread directoryThread (subscribeToDirectory, &directorySocket);
  boost::thread EPNThread (pushToEPN, &EPNSocket, fEventSize);

#ifndef DEBUGMSG
  boost::thread logThread (Log, &directorySocket, &EPNSocket);
#endif
  
  //Wait for the threads to finish execution
  directoryThread.join();
  EPNThread.join();

#ifndef DEBUGMSG
  logThread.join();
#endif
  
  return 0;
}
