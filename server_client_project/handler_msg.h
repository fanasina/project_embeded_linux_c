#ifndef __HANDLER_MSG__
#define __HANDLER_MSG__

#include <pthread.h>
#include "tools.h"

void handle_message(char * message, char **out);
void* seuil_temp(void*arg);
void * freq_log(void* arg);

#endif
