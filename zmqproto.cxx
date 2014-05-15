#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include <sys/ioctl.h> //For ioctl
#include <net/if.h>
#include <netinet/in.h>

#include "zmqproto.h"


/*
int utilities_get_my_ip (const char * device)
{
  int fd;
  struct ifreq ifr;

  fd = socket (AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET; 
  strncpy (ifr.ifr_name, device, IFNAMSIZ-1);

  ioctl (fd, SIOCGIFADDR, &ifr);

  close (fd);
  return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}
*/

char *determine_ip()
{
  FILE *fp = popen("ifconfig", "r");
  
  if (fp) {
    char *p=NULL, *e;
    size_t n;
    while ((getline(&p, &n, fp) > 0) && p) {
      if (p = strstr(p, "inet ")) {
        p+=5;
        if (p = strchr(p, ':')) {
          ++p;
          if (e = strchr(p, ' ')) {
            *e='\0';
          }
          //Break at the first occurrence of a non-localhost ip
          if (! strstr(p, "127.0.0.1")) {
              break;
          }
        }
      }
    }
    pclose(fp);
    return p;
  }
  else {
    return NULL;
  }
}

void *publisher (void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  pthread_t self_id = pthread_self();
  
  int fEventSize = 10000;
  
  Content* payload = new Content[fEventSize];
  for (int i = 0; i < fEventSize; ++i) {
        (&payload[i])->id = 0;
        (&payload[i])->x = rand() % 100 + 1;
        (&payload[i])->y = rand() % 100 + 1;
        (&payload[i])->z = rand() % 100 + 1;
        (&payload[i])->a = (rand() % 100 + 1) / (rand() % 100 + 1);
        (&payload[i])->b = (rand() % 100 + 1) / (rand() % 100 + 1);
  }
  
  int counter = 0;
  
  //Initialize publisher socket
  zmq::socket_t publisher(*context, ZMQ_PUSH);
  publisher.connect("tcp://localhost:5556");
  
  //Send messages to the proxy, 1 per second
  while (1) {
    zmq::message_t msg (fEventSize * sizeof(Content));
    memcpy(msg.data(), payload, fEventSize * sizeof(Content));
    publisher.send (msg);
    
    cout << self_id << " " << determine_ip() << " -> Sent message " << counter++ << endl;
    
    sleep(1);
  }
}

void *subscriber (void *arg)
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
    subscriber.recv (&msg); // First, discard the Dealer header (http://stackoverflow.com/questions/16692807/)
    subscriber.recv (&msg);
    
    Content* input = reinterpret_cast<Content*>(msg.data());

    cout << self_id << " " << determine_ip() << " " << (&input[0])->x << " " << (&input[0])->y << " " << (&input[0])->z << " " << (&input[0])->a << " " << (&input[0])->b << " Received message " << counter++ << endl;
  }
}

int main(int argc, char** argv)
{
  if ( argc != 3 ) {
    cout << "Usage: zmqproto\tnumFLP\tnumEPN" << endl;
    return 1;
  }
  
  int numFLP = atoi(argv[1]);
  int numEPN = atoi(argv[2]);
  
  if ( numFLP <= 0 || numEPN <= 0 ) {
    cout << "Error: At least 1 FLP and EPN needed\n" << endl;
    return -1;
  }
  
  //Initialize zmq
  zmq::context_t context (1);
    
  //Start publisher on a new thread
  for ( int i = 0; i < numFLP; i++ ) {
    pthread_t publisherThread;
    pthread_create (&publisherThread, NULL, publisher, &context);
  }
  
  //Start subscriber on a new thread
  for ( int i = 0; i < numEPN; i++ ) {
    pthread_t subscriberThread;
    pthread_create (&subscriberThread, NULL, subscriber, &context);
  }
  
  //Block on proxy
  zmq::socket_t frontend (context, ZMQ_ROUTER);
  frontend.bind("tcp://*:5556");

  zmq::socket_t backend (context, ZMQ_DEALER);
  backend.bind("tcp://*:5558");

  //  Initialize poll set
  zmq::pollitem_t items [] = {
    { frontend, 0, ZMQ_POLLIN, 0 },
    { backend,  0, ZMQ_POLLIN, 0 }
  };
  
  vector<string>ipVector;
  vector<string>::iterator ipVectorIter;

  cout<<"Vector size: "<<ipVector.size()<<endl;
  
  for ( long vectorIndex = 0; vectorIndex < (long)ipVector.size(); vectorIndex++ ) {
    cout <<vectorIndex<<" - "<<ipVector.at(vectorIndex)<<endl;
  }
  
  //  Switch messages between sockets
  while (1) {
    zmq::message_t message;
    int64_t more;           //  Multipart detection

    zmq::poll (&items [0], 2, -1);

    if (items [0].revents & ZMQ_POLLIN) {
      while (1) {
        //  Process all parts of the message
        frontend.recv(&message);
        
        size_t more_size = sizeof (more);
        frontend.getsockopt(ZMQ_RCVMORE, &more, &more_size);
        backend.send(message, more? ZMQ_SNDMORE: 0);

        if (!more)
          break; //  Last message part
      }
    }
    if (items [1].revents & ZMQ_POLLIN) {
      while (1) {
        //  Process all parts of the message
        backend.recv(&message);
        
        size_t more_size = sizeof (more);
        backend.getsockopt(ZMQ_RCVMORE, &more, &more_size);
        frontend.send(message, more? ZMQ_SNDMORE: 0);
        
        if (!more)
          break; //  Last message part
      }
    }
    
    //Add the IP to the IP vector but only if it is not already in
    ipVectorIter = find (ipVector.begin(), ipVector.end(), determine_ip());
    
    if ( ipVectorIter == ipVector.end() ) {
      //FIXME: performance wise is better to allocate big blocks of memory for the vector instead of element-by-element with each push_back()
      ipVector.push_back(determine_ip());
    }
    
    cout<<"Vector size: "<<ipVector.size()<<endl;
    
    for ( long vectorIndex = 0; vectorIndex < (long)ipVector.size(); vectorIndex++ ) {
      cout <<vectorIndex<<" - "<<ipVector.at(vectorIndex)<<endl;
    }
  }
  
  return 0;
}

