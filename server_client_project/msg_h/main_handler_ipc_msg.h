#ifndef __MAIN_HANDLER_IPC_MSG__
#define __MAIN_HANDLER_IPC_MSG__

#include "head_srv_cli.h"

struct arg{
  int msgid;
  long int canal;
};


void* msg_handler(void* args); 

#endif
