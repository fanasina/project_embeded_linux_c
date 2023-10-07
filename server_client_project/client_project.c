/*
 * client
 * */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#include "tools.h"

#define SERVER_ADDR  "192.168.75.140"   /*"134.119.191.253"*/ /*"192.168.1.88"*/
#define PORT 12351

//#define SIZE_BUF 256



int sock;
int end_cli=0;
pthread_mutex_t mut_sock;

void usage(){
  printf("usage:\n [FORMAT] \t\t<device_num>:<action>[:<args>]\n\n\t");
  printf("<device_num>: \n\n\t\t\t"); 
  printf("%d (:leds),         %d (:button),           %d (:lcd), \n\n\t\t\t",MOD_LED, MOD_INT, MOD_LCD); 
  printf("%d (:sensor),       %d (:log file update),  %d (:main server to setup some variables)\n\n\t", MOD_SENSOR, LOG, MAIN_SERV);
  printf("<action>:\n\t\t");
  printf("w,  write:          to write on device\n\t\t");
  printf("r,  read:           to read device\n\t\t");
  printf("l,  lcdirectread:   to read on <device> and then display result on lcd\n\t");
  printf("<args>:\n\t|    \n\t");
  printf("|-if <action>=  w,  write:\n\t|   |\n\t");
  printf("|   |-if <dev>=%3d (:leds):\n\t",MOD_LED);
  printf("|   |                             'on','off','push','altern','ledelay:DELAY' for all leds,\n\t");
  printf("|   |                             'oni', 'offi','pushi','ledelayi:DELAY', for each led/buzzer N° i, DELAY number in millisecond\n\t|   |\n\t");
  printf("|   |-if <dev>=%3d (:lcd):\n\t",MOD_LCD);
  printf("|   |                             '0msg' to display 'msg' on first line, '1msg' to display 'msg' on second line,\n\t"); 
  printf("|   |                             'msg' to display on all screen\n\t");
  printf("|   |                             'refreshi:REFRESH' to change refresh on line i=0,1 to REFRESH number in milliseconds,\n\t");
  printf("|   |                             'refresh:REFRESH' to change all line refresh\n\t|   |\n\t");
  printf("|   |-if <dev>=%3d (:main_serv):  '[freqlog|freqcheck] :FREQ' FREQ in microseconds, 'seuil':TEMP°C\n\t|   |\n\t",MAIN_SERV);
  printf("|   |-if <dev>=%3d (:log):         no arg:  read sensor and print log in file log and lcd\n\t|    \n\t",LOG);
  printf("|-if <action>=  r,  read:          \n\t");
  printf("    |-if <dev>=%3d (:log):        [ARG][optional] to display the last log on lcd, and the last ARG+1 logs on client \n\t    |\n\t",LOG);
  printf("    |-for devices 1,2,3,4,6       [no arg ] [it displays the args (syntaxe) to update in write <action>]\n>");
  /*
  printf("    |-if <dev>=%3d (:lcd):         no args to display refress\n\t    |\n\t",MOD_LCD);
  printf("    |-if <dev>=%3d (:main_serv):   no args to display freq\n\t    |\n\t",MAIN_SERV);
  printf("    |-if <dev>=%3d (:log):         no args to display the last log in file\n\t    |\n\t",LOG);
  printf("    |-if <dev>=%3d (:sensor):      no arg to receive ambiante temperature and humidity\n>",MOD_SENSOR);
*/
}


void* receive_msg(void*){
  char buf[SIZE_BUF];
  int ret;
  while(end_cli == 0){
    pthread_mutex_lock(&mut_sock); 
    ret = read(sock, buf, SIZE_BUF); /* blocant -> */
    pthread_mutex_unlock(&mut_sock); 
    if(ret>0){ /* blocant -> */
      buf[ret]='\0';
      printf("%s\n>\n", buf);
    }
    
    if(strncmp(buf,"quit",4)==0 || strncmp(buf,"exit",4)==0 ){ //
      close(sock);
      exit(0);

      printf("entrer n'importe quel caractère pour sortir\n");
      end_cli=1;
      break;
    }
    
  }
}

int main() {
  pthread_mutex_init(&mut_sock, NULL);
  sock = socket(AF_INET , SOCK_STREAM ,0);
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT); // Port du serveur
  addr.sin_addr.s_addr = inet_addr(SERVER_ADDR); // Adresse IP du serveur (localhost)
  int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
  if(res==-1) { perror("connect"); exit(1); }
  
  int flags = fcntl(sock, F_GETFL, 0); /*  pour recuperer le flag de sock */
  flags = flags | O_NONBLOCK;
  fcntl(sock, F_SETFL, flags);


  char buf[256];
    int ret_read;
    pthread_mutex_lock(&mut_sock);
    ret_read = read(sock, buf, SIZE_BUF); /* blocant -> */
    pthread_mutex_unlock(&mut_sock);
    if(ret_read>0){ /* blocant -> */
      printf("%s\n", buf);
    }

  pthread_t thrd_receive;
  pthread_create(&thrd_receive,NULL, receive_msg, NULL);
  do{
    printf("enter your cmd :\n\tquit: to quit this client\n\texit: to kill server and all clients\n\thelp: to display help/usage): \n>");
    
    scanf("%s",buf);
    
    if(buf[0] =='h') { 
      usage();
    }
    else{
    
      pthread_mutex_lock(&mut_sock);
      write(sock, buf, strlen(buf));
      pthread_mutex_unlock(&mut_sock);
    }
  }while(strcmp(buf,"quit") != 0 && strcmp(buf,"exit") != 0  && end_cli==0);
  
  end_cli = 1;


  pthread_join(thrd_receive, NULL);

  close(sock);

  pthread_mutex_destroy(&mut_sock);
  return EXIT_SUCCESS;

}
