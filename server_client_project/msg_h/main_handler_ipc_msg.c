

#include "main_handler_ipc_msg.h"


void* msg_handler(void* args){
  struct arg *argm=(struct arg*)args;
  int msgid = argm->msgid;
  long int canal= argm->canal;
//void boucle(int msgid,int canal){
    struct Message message;
    int result;

    struct MessageAnswer msgAnswer;
    struct msqid_ds info;

   while(1){
        //printf("sizeof(message):%ld - sizeof(message.mtype:%ld =%ld\n",sizeof(message), sizeof(message.mtype),sizeof(message) - sizeof(message.mtype)); 
	   
          if (msgrcv(msgid, &message, sizeof(message) - sizeof(message.mtype), canal, 0) == -1)
          {
                perror("Error receiving the question");
                exit(1);
          }

        printf("MSG received: dev_num:%d, action:%s, dev_name:%s\n msg:%s\n", message.dev_num, message.action, list_dev[message.dev_num], message.msg);
       
        msgAnswer.result = result;
        msgAnswer.mtype = message.pid;
        sprintf(msgAnswer.msg, "answer:%s", message.msg);
        if (msgsnd(msgid, &msgAnswer, sizeof(struct MessageAnswer) - sizeof(msgAnswer.mtype), 0) == -1)
        {
          perror("Error while sending the answer");
          exit(1);
        }

        printf("Answer sent: %d\n", result);
   }
}


