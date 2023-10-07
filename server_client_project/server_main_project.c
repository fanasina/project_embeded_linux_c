/*
 * server main
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>


#include <fcntl.h>

#include <pthread.h>

#include <signal.h>

#include "tools.h" 
#include "handler_msg.h"

//#define SIZE_BUF 256
//#define SIZE_PSEUDO 25

#define PORT 12351

//#define MAX_CONN 10


int sock;

pthread_mutex_t mut_client[MAX_CONN];
pthread_mutex_t mut_fd[COUNT_FILES];


int client[MAX_CONN];

char pseudo[MAX_CONN][SIZE_PSEUDO];

int nb_clients=0;

int end_log=0;
int end_check=0;
// ======================================================
//
struct sigaction old;


void close_all(){
  for(int i=0; i<nb_clients; ++i)
    close(client[i]);
  close(sock);
  end_log=1;
  end_check=1;
}


void func_sig_Handler(int sig){
  if(sig==SIGINT){
    char message[SIZE_BUF + SIZE_PSEUDO + 13];

    sprintf(message,"%s:from:server","exit");
    for(int j=0; j<nb_clients; ++j){
        pthread_mutex_lock(&mut_client[j]);
        write(client[j], message, strlen(message));
        pthread_mutex_unlock(&mut_client[j]);
    }
    close_all();
    sigaction(SIGINT, &old, NULL); /* equivalent à .sa_falg =  SA_RESETHAND , i.e. 1 seul new_action, puis on revient à l'ancien*/
    
    exit(0);
  }
}


// ============================================

void* handle_client(void* data){

    int identifiant=*(int*)data;
  
    printf("client N°:%d\n", identifiant);

    char buf[SIZE_BUF];

    char message[SIZE_BUF + SIZE_PSEUDO + 13];
    size_t len;
    
    int dst;
    char msg[SIZE_BUF];
    char *out=NULL;

    sprintf(pseudo[identifiant],"client:%d", identifiant);
    
    sprintf(message,"you_are:%s",pseudo[identifiant]);

    write(client[identifiant], message, strlen(message));
    
    //handle_message(message);

    while(1){
      /*if((*/ len = read(client[identifiant], buf, SIZE_BUF);/*) > 0){*/
        buf[len]='\0';
        printf("reçu: %s\n", buf);
        
        handle_message(buf, &out);
        
        if(out == NULL) sprintf(message,"%s:from:%s",buf,pseudo[identifiant]);
        else sprintf(message,"result:%s",out);
       
        printf("<message>%s\n",message);
         
        for(int j=0; j<nb_clients; ++j){
          pthread_mutex_lock(&mut_client[j]);
          //if(j!=identifiant  && client[j]>= 0 ){
            write(client[j], message, strlen(message));
          //}    
          pthread_mutex_unlock(&mut_client[j]);
        }
        if(strcmp(buf,"quit") == 0)
          break;
        if(strcmp(buf,"exit") == 0){
          close_all();
          exit(0);
          break;
        }
        if(out!=NULL) {
          free(out);
          out=NULL;
        }
      //}
    }
   
    pthread_mutex_lock(&mut_client[identifiant]);
    close(client[identifiant]);

    client[identifiant] = -1;
    pthread_mutex_unlock(&mut_client[identifiant]);
    // is_active[i]=0;

}

int main() {

  struct sigaction new_action;

  new_action.sa_handler = func_sig_Handler;
 // new_action.sa_flags = SA_RESETHAND; /* pour que le handler custom s'execute 1 seul fois */
  sigaction (SIGINT, &new_action, &old);

// =================================


  sock = socket(AF_INET , SOCK_STREAM , 0);
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT); // Port sur lequel le serveur écoute
  addr.sin_addr.s_addr = INADDR_ANY; // Écoute sur toutes les interfaces disponibles
  int res = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
  if (res == -1) { perror("bind"); exit(1); }
  //listen(sock, SOMAXCONN);
  listen(sock, MAX_CONN);
 // ============================= 
  pthread_t thrd_freq_log;
  end_log=0;
  pthread_create(&thrd_freq_log, NULL, freq_log,NULL);

  pthread_t thrd_check_tmp;
  end_check=0;
  pthread_create(&thrd_check_tmp, NULL, seuil_temp,NULL);



//==========================  

  pthread_t thrd_client[MAX_CONN];

  for(int i = 0; i<MAX_CONN; ++i){
    pthread_mutex_init(&mut_client[i], NULL);
  }
  for(int i = 0; i<COUNT_FILES; ++i){
    pthread_mutex_init(&mut_fd[i], NULL);
  }
  for(int i=0; i<MAX_CONN ; ++i){
     client[nb_clients] = accept(sock, NULL, NULL); /* blocant */
     int id_client = nb_clients;
     pthread_create(&thrd_client[nb_clients],NULL, handle_client, (void*)&id_client);
     ++nb_clients;  
  }
  
   for(int i=0; i< nb_clients; ++i){
    pthread_join(thrd_client[i], NULL);
  }

  end_log=1;
  end_check=1;
  pthread_join(thrd_freq_log,NULL);
  pthread_join(thrd_check_tmp,NULL);
  
  for(int i = 0; i<MAX_CONN; ++i){
    pthread_mutex_destroy(&mut_client[i]);
  }
  close(sock);
  
  for(int i = 0; i<COUNT_FILES; ++i){
    pthread_mutex_destroy(&mut_fd[i]);
  }
  return EXIT_SUCCESS;
}
