/*
 * messenger 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#include <pthread.h>

#define SIZE_BUFFER 128

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
};

// ========================================
struct Module{
  char dev_name[SIZE_BUFFER]; /* /dev/project_leds for instance */
  int action; /*write:1, read:0*/
  char args[SIZE_BUFFER];
  size_t pid;
};

struct Message_Module
{
  long mtype; /* channel */
  struct Module mod;
};

struct Message_Answer
{
	long mtype;
	int result;
  char msg_result[SIZE_BUFFER];
};

struct Message_only {
    long mtype;
    char msg[SIZE_BUFFER];
};



// =====================================

struct MessageAnswer
{
	long mtype;
	int result;
};

void* handle_msg(void* arg){
    int msgid = *(int*)arg;
    struct Message_Module msg_mod;

    struct Message_Answer answer;

    // char continueLoop = 'y';
    // while (continueLoop == 'y' || continueLoop == 'Y')
    do
    {
      printf("Waiting module info ...\n");

       //if (msgrcv(msgid, &msg_mod, sizeof(struct Message_Module) - sizeof(msg_mod.mtype), 1, 0) == -1)
       if (msgrcv(msgid, &msg_mod, sizeof(struct Module), 1, 0) == -1)
        {
           perror("Error receiving the module info");
           exit(1);
        }
/*
      printf("your dev_name:\n");

        scanf("%s", message.mod.dev_name);

        printf("Enter args: ");
        scanf("%s", message.mod.args);

        //message.question = question;
*/

      answer.mtype = msg_mod.mod.pid;
      strcpy(answer.msg_result,msg_mod.mod.args);
        
      printf("Answer send: %s\n", answer.msg_result);
        if (msgsnd(msgid, &answer, sizeof(struct Message_Answer) - sizeof(answer.mtype), 0) == -1) {
            perror("Error sending the message module \n");
            exit(1);
        }

/*      if (msgrcv(msgid, &answer, sizeof(struct Message_Answer) - sizeof(answer.mtype), pid, 0) == -1)
      {
        perror("Error receiving the answer");
        exit(1);
      }
*/


    }while(strcmp(answer.msg_result, "quit")!=0);

}

int main() {
    key_t key = ftok("/opt/com", 'm');
   // key_t key = ftok(".", 'M');
    //int msgid = msgget(key, 0666);
    int msgid = msgget(key, IPC_CREAT | 0666);
   printf("msgid=%d serv wait\n",msgid);

    if (msgid == -1) {
        perror("Error creating the message queue");
        exit(1);
    }

    pthread_t t_msg;
    pthread_create(&t_msg, NULL, handle_msg, (void*)&msgid);

    pthread_join(t_msg,NULL);

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Error deleting the message queue");
        exit(1);
    }


    exit(0);
}
