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

#include "tools.h"

#define SIZE_MSG 64
//#define SIZE_BUF 1024

#define DEST_CMD_NB 5

/* list cmd destination */
#define MOD_LED 0
#define MOD_INT 1 /* button */
#define MOD_LCD 2
#define MOD_SENSOR 3
#define LOG 4

#define MAIN_SERV 5
#define MAIN_HAND_MSG 6
#define CLIENT 7

char *list_dev[64]={
  "/dev/project_leds",
  "/dev/project_int_b",
  "/dev/project_lcd",
  "/dev/project_dht11",
  "/opt/log_temp",
};


struct Message {
    long mtype;
    pid_t pid;
    char action[SIZE_MSG];
    int dev_num; /* dest cmd types */
    //char dev_name[SIZE_MSG]; 
    char msg[SIZE_MSG];
};

struct MessageAnswer
{
  long mtype;
  int result;
  char msg[SIZE_MSG+64];
};

#endif
