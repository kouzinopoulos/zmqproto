#ifndef ZMQPROTOSUBSCRIBER_H
#define ZMQPROTOSUBSCRIBER_H

class zmqprotoSubscriber : public zmqprotoCommon
{
  public:
    static void *Run(void *arg);
};

#endif
