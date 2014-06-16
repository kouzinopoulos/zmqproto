#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <vector>

#include <time.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "zmqprotoCommon.h"
#include "zmqprotoContext.h"
#include "zmqprotoDirectory.h"
#include "zmqprotoMessage.h"
#include "zmqprotoSocket.h"

using namespace std;

char FLPIPAddr[30], EPNIPAddr[30];

vector<string>ipVector;
vector<string>::iterator ipVectorIter;

boost::mutex mtx_;

void Log (zmqprotoSocket *frontendPtr, zmqprotoSocket *backendPtr)
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
    
    cout << setprecision(2) << fixed
         << "EPNs: " << ipVector.size() << " "
         << "Tx \033[01;34m" << (frontendPtr->GetBytesTx() - bytesTx)/1048576 << " Mb/s "
         << frontendPtr->GetMessagesTx() - messagesTx << " msg/s \033[0m " << "Rx \033[01;31m"
         << (backendPtr->GetBytesRx() - bytesRx)/1048576 << " Mb/s "
         << backendPtr->GetMessagesRx() - messagesRx << " msg/s \033[0m "
         << "Avg Tx \033[01;34m" << frontendPtr->GetBytesTx()/(difTime * 1048576) << " Mb/s "
         << frontendPtr->GetMessagesTx()/difTime << " msg/s \033[0m Rx " 
         << "\033[01;31m" << backendPtr->GetBytesRx()/(difTime * 1048576) << " Mb/s "
         << backendPtr->GetMessagesRx()/difTime << " msg/s \033[0m " << '\r' << flush;
         
    bytesTx = frontendPtr->GetBytesTx();
    messagesTx = frontendPtr->GetMessagesTx();
    bytesRx = backendPtr->GetBytesRx();
    messagesRx = backendPtr->GetMessagesRx();
    
    boost::this_thread::sleep (boost::posix_time::seconds(1));
  }
}

void publishToFLP (zmqprotoSocket *fSocketPtr)
{
  //Bind for connections from FLPs
  fSocketPtr->Bind (FLPIPAddr);
  
  while (1) {
    if (ipVector.size() > 0) {
      //Pack the IP vector using msgpack and send it to all connected FLPs
      msgpack::sbuffer sbuf;
      msgpack::pack (sbuf, ipVector);

      zmqprotoMessage msg (sbuf.size());
      
      mtx_.lock();
      memcpy (msg.GetData(), sbuf.data(), sbuf.size());
      mtx_.unlock();

      fSocketPtr->Send (&msg, "");
    }
  }
}

void receiveFromEPN (zmqprotoSocket *fSocketPtr)
{
  //Bind for connections from EPNs
  fSocketPtr->Bind (EPNIPAddr);
  
  while (1) {
    zmqprotoMessage msg;
    fSocketPtr->Receive (&msg, "");
    
    PRINT << "Received a message with a size of " << msg.GetSize();
    
    //If the IP is unknown, add it to the IP vector
    ipVectorIter = find (ipVector.begin(), ipVector.end(), reinterpret_cast<char *>(msg.GetData()));
    
    if ( ipVectorIter == ipVector.end() ) {
      //FIXME: performance wise is better to allocate big blocks of memory for the vector instead of element-by-element with each push_back()
      mtx_.lock();
      ipVector.push_back (reinterpret_cast<char *>(msg.GetData()));
      mtx_.unlock();

      PRINT << "Unknown IP, adding it to vector. Total IPs: " << ipVector.size();
    }
  }
}

int main(int argc, char** argv)
{
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " publishToFLPIPPort receiveFromFLPIPPort" << endl;
    return 1;
  }
  
  if (atoi (argv[1]) > 65535 || atoi (argv[2]) > 65535 || atoi (argv[1]) < 1 || atoi (argv[2]) < 1) {
    cout << "Usage: " << argv[0] << " publishToFLPIPPort receiveFromFLPIPPort" << endl;
    return 1;
  }
  
  snprintf(FLPIPAddr, 30, "tcp://*:%s", argv[1]);
  snprintf(EPNIPAddr, 30, "tcp://*:%s", argv[2]);
  
  //Initialize zmq
  int numIoThreads = 1;
  zmqprotoContext fContext (numIoThreads);
  
  //Initialize sockets
  zmqprotoSocket FLPSocket (fContext.GetContext(), "pub", 0);
  zmqprotoSocket EPNSocket (fContext.GetContext(), "pull", 1);
  
  //Launch the threads that handle the sockets
  boost::thread FLPThread (publishToFLP, &FLPSocket);
  boost::thread EPNThread (receiveFromEPN, &EPNSocket);
  
#ifndef DEBUGMSG
  boost::thread logThread (Log, &FLPSocket, &EPNSocket);
#endif

  //Wait for the threads to finish execution
  FLPThread.join();
  EPNThread.join();
  
#ifndef DEBUGMSG
  logThread.join();
#endif  
  
  return 0;
}
