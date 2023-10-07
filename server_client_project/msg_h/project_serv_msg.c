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


int main() {
    key_t key = ftok("/opt/com", 'M');
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("Error creating the message queue");
        exit(1);
    }
   
    printf("msgid server:%d\n",msgid);

    pthread_t thx[DEST_CMD_NB];
    struct arg id_arg[DEST_CMD_NB];
    for(long int i=0; i<DEST_CMD_NB; ++i){
      id_arg[i].msgid = msgid;
      id_arg[i].canal = i;
      pthread_create(&thx[i],NULL, boucle,(void*)&id_arg[i]);
    }
   
    for(int i=0; i<DEST_CMD_NB; ++i){
      pthread_join(thx[i], NULL);
    } 

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Error deleting the message queue");
        exit(1);
    }


    exit(0);
}
