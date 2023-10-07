#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>

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
};

struct MessageAnswer
{
	long mtype;
	int result;
};

void boucle(int msgid,int canal){
    struct Message message;
    struct Answer answer;
   
    struct MessageAnswer msgAnswer;
    struct msqid_ds info;

   while(1){
	    sleep(10);

	    if(msgctl(msgid, IPC_STAT, &info) == -1){
		    perror("Error msgctl");
		    exit(1);
	    }	    
	    printf("il y %ld en attente \n", info.msg_qnum);
	    for(int i = 0; i < info.msg_qnum; ++i){
		    printf("il y a %ld messages en attente du canal %d \n", info.msg_qnum - i, canal);
	    
	   
	   
      if (msgrcv(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), canal, 0) == -1)
	    {
            perror("Error receiving the question");
            exit(1);
      }

      switch (message.operator) {
          case '+':
              answer.result = message.op1 + message.op2;
              break;
          case '-':
              answer.result = message.op1 - message.op2;
              break;
          case '*':
              answer.result = message.op1 * message.op2;
              break;
          case '/':
              answer.result = message.op1 / message.op2;
              break;
        case 'x':
          exit(1);

      }
	//sleep(1);
    printf("Question received: %d %c %d\n", message.op1, message.operator, message.op2);

    msgAnswer.result = answer.result;
    msgAnswer.mtype = message.pid;
    if (msgsnd(msgid, &msgAnswer, sizeof(struct MessageAnswer) - sizeof(msgAnswer.mtype), 0) == -1)
    {
		perror("Error while sending the answer");
		exit(1);
	}

	  printf("Answer sent: %d\n", answer.result);
        sleep(1);
      }
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

    pid_t pid_moins;

    pid_moins = fork();

    if(pid_moins == -1){
      perror("error fork");
      exit(1);
    }
    
    if(pid_moins == 0){ // fils moins
       boucle(msgid,2);


    }
    else{ // pere plus
      pid_t pid_mult;

      pid_mult = fork();

      if(pid_mult == -1){
        perror("error fork");
        exit(1);
      }
      if(pid_mult == 0){// fils multi
        boucle(msgid, 3);
      }
      else{// pere
        pid_t pid_div;

        pid_div = fork();

        if(pid_div == -1){
          perror("error fork");
          exit(1);
        }
        if(pid_div == 0){// fils divi
          boucle(msgid, 4);
        }
        
        boucle(msgid, 1);
        wait(NULL);
      }
    }
        


    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Error deleting the message queue");
        exit(1);
    }


    exit(0);
}
