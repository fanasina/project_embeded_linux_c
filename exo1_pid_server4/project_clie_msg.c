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

void *loop(void* arg){
    int msgid=*(int*)arg;

    struct Message message;
    long int pid_th = /*pthread_self(); */getpid();
    message.pid = pid_th; //getpid();

    struct MessageAnswer answer;
    answer.mtype = pid_th;

 
  while(1)
    {
        struct Question question;
        //
        printf("Your cmd: format \n\t\tdev_num:action:args\n\t\tdev_num lists: 0:leds, 1:button, 2:lcd\n\t\taction: w:write, r:read\n\t\t\targs leds:on,off,push,altern,ledelay:DELAY for all leds,\n\t\t\t for led i: oni, offi,pushi,ledelayi:DELAYi DELAY number in millisecond");
        scanf("%s",message.msg);/*cmd*/
        /* format */
        /* dev_num:action:args, eg 0:r: */
        

        message.action = getchar();

        if (message.action != 'w' && message.action != 'r')
        {
         
          for( int i = 1; i <= 4; ++i){
            message.mtype = i;
            //if (msgsnd(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), 0) == -1)		     {
            if (msgsnd(msgid, &message, sizeof(message) - sizeof(message.mtype), 0) == -1)		     {
                perror("Error sending quit msg action");
                exit(1);
            }
          }
          exit(0);

        }

                

        switch(message.action){
          case 'r':  message.mtype = 1; break;
          case 'w':  message.mtype = 2; break;
        }

        printf("dev_name:\n");
        scanf("%s",message.dev_name);
        printf("dev_num:

        printf("Enter operand 1: ");
        scanf("%d", &message.op1);

        //scanf(" %c", &message.operator);
        printf("Enter operand 2: ");
        scanf("%d", &message.op2);

        //message.question = question;

        if (msgsnd(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), 0) == -1) {
            perror("Error sending the question");
            exit(1);
        }

      printf("Waiting answer ...\n");
      if (msgrcv(msgid, &answer, sizeof(struct MessageAnswer) - sizeof(answer.mtype), getpid(), 0) == -1)
      {
        perror("Error receiving the answer");
        exit(1);
      }

      printf("Answer received: %d,\n msg received:%s\n", answer.result, answer.msg);


            //printf("Do you want to continue? (y/n): ");
            //scanf(" %c", &continueLoop);
    }

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
