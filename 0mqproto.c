#include "zhelpers.h"

void *publisher (void *context)
{
  //Initialize publisher
  void *publisher = zmq_socket (context, ZMQ_PUSH);
  int rc = zmq_connect (publisher, "tcp://localhost:5556");
  assert (rc == 0);
  
  //Send messages to the proxy, 1 per second
  while (1) {
    char update [6];
    sprintf (update, "Hello");
    s_send (publisher, update);
    
    sleep(1);
  }
  
  zmq_close (publisher);
  zmq_ctx_destroy (context);
}

void *subscriber (void *context)
{
  pthread_t self_id = pthread_self();
  
  //Initialize subscriber
  void *subscriber = zmq_socket (context, ZMQ_PULL);
  int rc = zmq_connect (subscriber, "tcp://localhost:5558");
  assert (rc == 0);
  
  //Receive messages from the subscriber
  while (1) {
    char *string = s_recv (subscriber);
  
    printf("%u: Received message: %s\n", self_id, string);
  }
  
  zmq_close (subscriber);
  zmq_ctx_destroy (context);
}

int main(int argc, char** argv)
{
  if ( argc != 3 ) {
    printf("Usage: 0mqproto\tnumFLP\tnumEPN\n");
    return 1;
  }

  //cout<<determine_ip()<<endl;
  
  int numFLP = atoi(argv[1]);
  int numEPN = atoi(argv[2]);
  
  if ( numFLP <= 0 || numEPN <= 0 ) {
   printf("Error: At least 1 FLP and EPN needed\n");
    return -1;
  }
  
  //Initialize zmq
  void *context = zmq_ctx_new ();

  int i;
  
  //Start publisher on a new thread
  for ( i = 0; i < numFLP; i++ ) {
    pthread_t publisherThread;
    pthread_create (&publisherThread, NULL, publisher, context);
  }
  
  //Start subscriber on a new thread
  for ( i = 0; i < numEPN; i++ ) {
    pthread_t subscriberThread;
    pthread_create (&subscriberThread, NULL, subscriber, context);
  }
  
  //Block on proxy
  void *frontend = zmq_socket (context, ZMQ_PULL);
  int rc = zmq_bind (frontend, "tcp://*:5556");
  assert (rc == 0);

  void *backend = zmq_socket (context, ZMQ_PUSH);
  rc = zmq_bind (backend, "tcp://*:5558");
  assert (rc == 0);
  
  zmq_proxy (frontend, backend, NULL);
  
  zmq_close (frontend);
  zmq_close (backend);
  zmq_ctx_destroy (context);
  
  return 0;
}

