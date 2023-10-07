#ifndef __HEAD_SRV_CLI_MOD__
#define __HEAD_SRV_CLI_MOD__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#include <pthread.h>

#define SIZE_MSG 64

#define NB_ACTION 2 /*read:0, write:1 */


struct Message {
    long mtype;
    pid_t pid;
    unsigned char action;
    int dev_num;
    char dev_name[SIZE_MSG];
    char msg[SIZE_MSG];
};

struct MessageAnswer
{
  long mtype;
  int result;
  char msg[SIZE_MSG+64];
};

#endif
