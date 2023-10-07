#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

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

struct MessageAnswer
{
	long mtype;
	int result;
};

int main() {
    key_t key = ftok("/opt/com", 'M');
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("Error creating the message queue");
        exit(1);
    }

    printf("msgid client:%d\n",msgid);

    struct Message message;
    message.pid = getpid();

    struct MessageAnswer answer;
    answer.mtype = getpid();

    // char continueLoop = 'y';
    // while (continueLoop == 'y' || continueLoop == 'Y')
    while(1)
    {
        struct Question question;
	//
	printf("Your question ?\n");
	printf("Enter operator (+, -, *, /) or x to quit: ");

        char c;
        do {
            c = getchar();

        } while (c != '+' && c != '-' && c != '/' && c != '*' && c != 'x');

	if (c == 'x')
	{
		message.operator = c;
    for( int i = 1; i <= 4; ++i){
      message.mtype = i;
		  if (msgsnd(msgid, &message, sizeof(struct Message) - sizeof(message.mtype), 0) == -1)		     {
       		perror("Error sending the question");
       		exit(1);
      }
    }
		exit(1);

	}

	message.operator = c;

  switch(c){
    case '+':  message.mtype = 1; break;
    case '-':  message.mtype = 2; break;
    case '*':  message.mtype = 3; break;
    case '/':  message.mtype = 4; break;
  }

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

	printf("Answer received: %d\n", answer.result);


        //printf("Do you want to continue? (y/n): ");
        //scanf(" %c", &continueLoop);
    }

    exit(0);
}
