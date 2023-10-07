/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>

#include <pthread.h>

#define SIZE_MSG 256

struct Question {
    int op1;
    int op2;
    char operator;
};

struct Answer {
    int result;
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

struct arg{
  int msgid;
  long int canal;
};

void* boucle(void* args){
  struct arg *argm=(struct arg*)args;
  int msgid = argm->msgid;
  long int canal= argm->canal;
//void boucle(int msgid,int canal){
    struct Message message;
    int result;

    struct MessageAnswer msgAnswer;
    struct msqid_ds info;

   while(1){
	    //sleep(10);

     /*
	    if(msgctl(msgid, IPC_STAT, &info) == -1){
		    perror("Error msgctl");
		    exit(1);
	    }
            
	    printf("il y %ld en attente \n", info.msg_qnum);
	    for(int i = 0; i < info.msg_qnum; ++i){
		    printf("il y a %ld messages en attente du canal %ld \n", info.msg_qnum - i, canal);
	    */
	  
        printf("sizeof(message):%ld - sizeof(message.mtype:%ld =%ld\n",sizeof(message), sizeof(message.mtype),sizeof(message) - sizeof(message.mtype)); 
	   
          //if (msgrcv(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), canal, 0) == -1)
          //if (msgrcv(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), canal, 0) == -1)
          if (msgrcv(msgid, &message, sizeof(message) - sizeof(message.mtype), canal, 0) == -1)
          {
                perror("Error receiving the question");
                exit(1);
          }

          switch (message.operator) {
              case '+':
                  result = message.op1 + message.op2;
                  break;
              case '-':
                  result = message.op1 - message.op2;
                  break;
              case '*':
                  result = message.op1 * message.op2;
                  break;
              case '/':
                  result = message.op1 / message.op2;
                  break;
            case 'x':
              exit(1);

          }
      //sleep(1);
        printf("Question received: %d %c %d\n msg received:%s\n", message.op1, message.operator, message.op2,message.msg);
       
        msgAnswer.result = result;
        msgAnswer.mtype = message.pid;
        sprintf(msgAnswer.msg, "answer:%s", message.msg);
        if (msgsnd(msgid, &msgAnswer, sizeof(struct MessageAnswer) - sizeof(msgAnswer.mtype), 0) == -1)
        {
          perror("Error while sending the answer");
          exit(1);
        }

        printf("Answer sent: %d\n", result);
            //sleep(1);
      //}
   }
}


int main() {
    key_t key = ftok("/opt/com", 'M');
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("Error creating the message queue");
        exit(1);
    }
   
    printf("msgid server:%d\n",msgid);

    pthread_t thx[4];
    struct arg id_arg[4];
    for(long int i=0; i<4; ++i){
      id_arg[i].msgid = msgid;
      id_arg[i].canal = i;
      pthread_create(&thx[i],NULL, boucle,(void*)&id_arg[i]);
    }
   
    for(int i=0; i<4; ++i){
      pthread_join(thx[i], NULL);
    } 

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Error deleting the message queue");
        exit(1);
    }


    exit(0);
}
