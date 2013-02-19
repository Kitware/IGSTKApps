#ifndef __ErrorManager_h
#define __ErrorManager_h

#include <fstream>
#include "igstkMacros.h"

#include "itkStdStreamLogOutput.h"

#define ERROR 0
#define SUCCESS 1

class ErrorManager
{
public:
  typedef unsigned int SUCCESS_VALUE;
  ErrorManager();
  void handleMessage(std::string, bool);
};

#endif
