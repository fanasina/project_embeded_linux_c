#include "handler_msg.h"

#define CRITICAL_TEMP 30
#define MAX_TEMP 52

#define FREQ_INIT (60000000) /*1 minute */
long int freq=FREQ_INIT;
extern int end_log;
extern int end_check;

int seuiltemp=CRITICAL_TEMP;
long int freqCheck=FREQ_INIT;


extern int client[MAX_CONN];


extern int nb_clients;
extern pthread_mutex_t mut_fd[COUNT_FILES];
extern pthread_mutex_t mut_client[MAX_CONN];

#define READ_(fd,dest,mode, buf)\
 do{\
      pthread_mutex_lock(&mut_fd[dest]);\
      int fd=open(list_dev[dest],mode);\
      if (fd == -1) {\
          perror(" error open to read file\n");\
          exit(1);\
      }\
      int ret = read(fd,buf,SIZE_BUF);\
      buf[ret]='\0';\
      close(fd);\
      pthread_mutex_unlock(&mut_fd[dest]);\
  }while(0);


#define WRITE_(fd,dest,mode,message)\
  do{\
      pthread_mutex_lock(&mut_fd[dest]);\
      int fd=open(list_dev[dest],mode );\
      if (fd == -1) {\
          perror(" error open to write file\n");\
          exit(1);\
      }\
      write(fd,message,strlen(message));\
      close(fd);\
      pthread_mutex_unlock(&mut_fd[dest]);\
  }while(0);

#define SIZE_LINE_LOG 35

void handle_message(char * message, char **out);

void* seuil_temp(void*arg){
  char msg[SIZE_BUF];
  int tempNow;
  //long freqq;
  do{
    READ_(fd,MOD_SENSOR,O_RDONLY,msg);
    tempNow=parse_field_char_to_lint(msg,1);
    printf("======seuil:%d===========temp_now = %d\n",seuiltemp,tempNow);
    if(tempNow > seuiltemp){
      //freqq=FREQ_INIT/(tempNow - seuiltemp);
      //sprintf(msg,"ledelay:%ld",freqq);
      //WRITE_(fd0,MOD_LED,O_WRONLY,msg);
      WRITE_(fd0,MOD_LED,O_WRONLY,"off");
      WRITE_(fd1,MOD_LED,O_WRONLY,"altern");
      WRITE_(fd2,MOD_LED,O_WRONLY,"on");
    }
    else{
      WRITE_(fd0,MOD_LED,O_WRONLY,"off");
    }
    usleep(freqCheck);
  }while(end_check==0); 
}


void* freq_log(void *arg){
  char msg[SIZE_BUF];
  char *out=NULL;
  printf("enter in free log func thread, freq=%ld\n",freq);
  do{
    usleep(freq);
    sprintf(msg,"%d:w:",LOG);
    printf("freq log: msg=%s\n",msg);
    handle_message(msg,&out);
    if(out!=NULL){
      WRITE_(fd,MOD_LCD,O_WRONLY,out);
      free(out);
      out=NULL;
    }

  }while(end_log==0);

}

void handle_message(char * message, char **out){
  int dst=parse_field_char_to_lint(message,0);

  char action[SIZE_BUF];
  strcpy(action,extract_field_char_btw_colon(message,1));
  char args[SIZE_BUF];
  strcpy(args,extract_field_from_to_btw_colon(message,2,5));
  printf("arg : %s\n",args);
  char to_exec[2*SIZE_BUF];
  char buf[SIZE_BUF];
  int ret;
  if(dst && dst < LOG){
    if(action[0]=='r'){
      READ_(fd,dst,O_RDONLY,buf);
      *out=malloc(strlen(buf));
      strcpy(*out,buf);
      sprintf(to_exec,"read(%s,buf,SIZE_BUF): out:%s\n",list_dev[dst],*out);
    }
    else if(action[0]=='w'){
      WRITE_(fd,dst,O_WRONLY,args);
      sprintf(to_exec,"write(%s,%s,strlen(args)\n",list_dev[dst],args);
    }
    else if(action[0]=='l'){
      READ_(fd,dst,O_RDONLY,buf);
      *out=malloc(strlen(buf));
      strcpy(*out,buf);
      sprintf(to_exec,"read(%s,buf,SIZE_BUF): out:%s\n",list_dev[dst],*out);
      WRITE_(fdlcd,MOD_LCD,O_WRONLY,buf);

    }
    printf("to exec:%s\n",to_exec);
  }
  else if (dst == LOG){// to log
    if(action[0]=='w'){// write to history log
      READ_(fd,MOD_SENSOR,O_RDONLY,buf);
      
      char *date=NULL;
      date=date_now();
      
      char msg[2*SIZE_BUF];
      sprintf(msg,"%s %s\n",date,buf);
      WRITE_(fdlog,LOG,O_WRONLY | O_APPEND,msg);
      WRITE_(fdlcd0,MOD_LCD,O_WRONLY,buf);
      sprintf(msg,"1%s",date);
      WRITE_(fdlcd1,MOD_LCD,O_WRONLY,msg);
      free(date);
    }
    else if(action[0] == 'r'){/* read log seulement, sans aller au sensor */
      int fd=open(list_dev[LOG],O_RDONLY);
      long position;
      char logg[SIZE_LINE_LOG];
      //printf("position 0=%ld\n",position);
      position=lseek(fd,-SIZE_LINE_LOG,SEEK_END);
      //printf("position size=%ld\n",position);
      long  count=parse_field_char_to_lint(args,1);
      long initcount=count, cur=0;
      
      do{
        ret=read(fd,logg,SIZE_LINE_LOG);
        logg[ret]='\0';
        if(count==initcount){
          WRITE_(fdlcdd,MOD_LCD,O_WRONLY,logg);
          //*out=malloc(strlen(logg));
          //strcpy(*out,logg);
        }
        for(int j=0; j<nb_clients; ++j){
          pthread_mutex_lock(&mut_client[j]);
          write(client[j], logg, strlen(logg));
          pthread_mutex_unlock(&mut_client[j]);
        }
 
        //printf("%ld: %s\n",initcount-count,logg);
        --count;
        position=lseek(fd,-2*SIZE_LINE_LOG,SEEK_CUR);
      }while(count >= 0 && position !=0 );

       
    }
  }
  else if(dst == MAIN_SERV){// loop
    if(action[0]=='w'){
      char msg[SIZE_BUF];
      strcpy(msg,extract_field_char_btw_colon(args,0));
      if(strcmp("freqlog",msg)==0){
        long int freqTemp=parse_field_char_to_lint(args, 1);
        if(freqTemp > 0){
          freq=freqTemp;
        }
      }
      else if(strcmp("freqcheck",msg)==0){
        long int freqchk=parse_field_char_to_lint(args, 1);
        if(freqchk > 0){
          freqCheck=freqchk;
        }
      }
      else if(strcmp("seuil",msg)==0){
        long int seuil=parse_field_char_to_lint(args, 1);
        if(seuil > 0 && seuil < MAX_TEMP){
          seuiltemp=seuil;
        }
      }
    }
    else if (action[0]=='r'){
      char msg[SIZE_BUF];
      sprintf(msg,"freqlog:%ld|seuil:%d|freqcheck:%ld",freq,seuiltemp,freqCheck);
      *out=malloc(strlen(msg));
      strcpy(*out,msg);

      WRITE_(fd,MOD_LCD,O_WRONLY,msg);
    }
  }

    /*
  for(int i = 0; i<3; ++i){
    printf("int in field[%d] = %ld \n",i,parse_field_char_to_lint(message,i));
  }

  for(int i = 0; i<3; ++i){
    printf("field[%d] = %s\n",i,extract_field_char_btw_colon(message,i));
  }

  for(int i = 0; i<3; ++i){
    printf("field[0] to field[%d]  = %s\n",i,extract_field_from_to_btw_colon(message,0,i));
  }
  for(int i = 0; i<3; ++i){
    printf("field[1] to field[%d]  = %s\n",i,extract_field_from_to_btw_colon(message,1,i));
  }
  */


}


