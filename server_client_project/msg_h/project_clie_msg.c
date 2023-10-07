/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#include <pthread.h>

#define SIZE_MSG 256



struct Question {
    int op1;
    int op2;
    char operator;
};

struct Message {
    long mtype;
    int op1;
    int op2;
    char operator;
    pid_t pid;
    char msg[SIZE_MSG];
};

struct MessageAnswer
{
	long mtype;
	int result;
  char msg[SIZE_MSG+64];
};

*/

#include "head_srv_cli.h"

void usage(){
  
  printf("Your cmd: format \ndev_num:action:args\ndev_num lists: 0:leds, 1:button, 2:lcd");
}

void *loop(void* argvoid){
    int msgid=*(int*)argvoid;

    struct Message message;
    long int pid_th = /*pthread_self(); */getpid();
    message.pid = pid_th; //getpid();

    struct MessageAnswer answer;
    answer.mtype = pid_th;
    char arg[SIZE_MSG];
 
  //while(1)
    do{
        usage();
        scanf("%s",message.msg);/*cmd*/
        /* format */
        /* dev_num:action:args, eg 0:r: */
        
        message.dev_num=parse_field_char_to_lint(message.msg,0);

        strcpy(message.action, extract_field_char_btw_colon(message.msg,1));

        if (message.action[0] != 'w' && message.action[0] != 'r')
        {
          strcpy(message.msg, "exit");
         
          for( int i = 1; i <DEST_CMD_NB; ++i){
            message.mtype = i;
            //if (msgsnd(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), 0) == -1)		     {
            if (msgsnd(msgid, &message, sizeof(message) - sizeof(message.mtype), 0) == -1)		     {
                perror("Error sending quit msg action");
                exit(1);
            }
          }
          exit(0);

        }

        if(message.action[0] == 'w'){ 
          strcpy(arg,extract_field_from_to_btw_colon(message.msg, 2,5));
          printf("write(fd[%d],args:%s, len...\n",message.dev_num,arg);
          strcpy(message.msg, arg);
        }
                
        message.mtype = message.dev_num;

        if (msgsnd(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), 0) == -1) {
            perror("Error sending the question");
            exit(1);
        }

      printf("Waiting answer ...\n");
      if (msgrcv(msgid, &answer, sizeof(struct MessageAnswer) - sizeof(answer.mtype), pid_th, 0) == -1)
      {
        perror("Error receiving the answer");
        exit(1);
      }

      printf("Answer received: %d,\n msg received:%s\n", answer.result, answer.msg);

      strcpy(arg, extract_field_char_btw_colon(answer.msg,0));
    }while(strcmp(arg,"exit") != 0);

}


int main() {
    key_t key = ftok("/opt/com", 'M');
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("Error creating the message queue");
        exit(1);
    }

    printf("msgid client:%d\n",msgid);

    // char continueLoop = 'y';
    // while (continueLoop == 'y' || continueLoop == 'Y')

    pthread_t th0;

    pthread_create(&th0, NULL, loop, (void*)&msgid);

    pthread_join(th0, NULL);

    exit(0);
}
