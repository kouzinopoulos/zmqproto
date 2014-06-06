#include <string>
#include <stdio.h>

#include "zmqprotoCommon.h"

using std::string;
using std::cout;
using std::endl;

zmqprotoCommon::~zmqprotoCommon()
{
#ifdef DEBUGMSG
  cout << os.str() << endl;
#endif
}

std::ostringstream& zmqprotoCommon::Print()
{
  os << "[" << "\033[01;34mDEBUG\033[0m" << "]" << " ";

  return os;
}
