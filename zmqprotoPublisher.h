#ifndef ZMQPROTOPUBLISHER_H
#define ZMQPROTOPUBLISHER_H

class zmqprotoPublisher : public zmqprotoCommon
{
  public:
    static void *Run(void *arg);
    
};

#endif
