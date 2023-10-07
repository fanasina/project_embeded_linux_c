#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include <string.h>

#include <pthread.h>

#define SIZE_BUFFER 128

// ========================================
struct Message_Answer{
  long mtype;
  int result;
  char msg_result[SIZE_BUFFER];
};

struct Module{
  char dev_name[SIZE_BUFFER]; /* /dev/project_leds for instance */
  int action; /*write:1, read:0 */
  char args[SIZE_BUFFER];
  size_t pid;
};

struct Message_Module
{
  long mtype; /* channel */
  struct Module mod;
};

// =====================================



void* thread_handler(void* arg){
  int msgid =*(int*)arg;
  size_t pid_thread=pthread_self();

  struct Message_Module msg_mod;
  struct Message_Answer msg_answer;

  do{
        printf("your dev_name:\n");

        scanf("%s", msg_mod.mod.dev_name);

        printf("Enter args: ");
        scanf("%s", msg_mod.mod.args);

        //message.question = question;
        msg_mod.mtype = 1;
        msg_mod.mod.pid=pid_thread;

        printf("msgid:%d, msg to send  mtype:%ld dev_name:%s, action:%d, args:%s, pid:%ld\n",msgid,msg_mod.mtype,msg_mod.mod.dev_name,msg_mod.mod.action,msg_mod.mod.args,msg_mod.mod.pid);
        
        //if (msgsnd(msgid, &msg_mod, sizeof(struct Message_Module) - sizeof(msg_mod.mtype), 0) == -1) {
        if (msgsnd(msgid, &msg_mod, sizeof(msg_mod.mod) , 0) == -1) {
        //if (msgsnd(msgid, &msg_mod, sizeof(struct Module) , 0) == -1) {
            perror("Error sending the message module \n");
            exit(1);
        }


       printf("wait for answers...\n");

    if (msgrcv(msgid, &msg_mod, sizeof(struct Message_Module) - sizeof(msg_mod.mtype), pid_thread, 0) == -1)
    {
       perror("Error receiving the module info");
       exit(1);
    }
    
    /*
    strcpy(msg_answer.msg_result, msg_mod.mod.dev_name);
    msg_answer.mtype = msg_mod.mod.pid;
    
    if (msgsnd(msgid, &msg_answer, sizeof(struct Message_Answer) - sizeof(msg_answer.mtype), 0) == -1)
    {
      perror("Error while sending the answer to module");
      exit(1);
    }*/


  }while(strcmp(msg_mod.mod.args,"quit") != 0);

}




int main() {
    key_t key = ftok("/opt/com", 'm');
    //key_t key = ftok(".", 'M');
    /*int msgid = msgget(key, IPC_CREAT | 0666);*/
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("Error creating the message queue");
        exit(1);
    }
    
    printf("msqid in to main:%d  \n",msgid);
            
    pthread_t th0;
    
    pthread_create(&th0, NULL, thread_handler,(void*)&msgid );

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Error deleting the message queue");
        exit(1);
    }

    pthread_join(th0,NULL);

    exit(0);
}
