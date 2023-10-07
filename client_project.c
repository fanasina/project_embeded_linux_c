#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define SIZE_BUFF 256
int main() {
  int sock = socket(AF_INET , SOCK_STREAM ,0);
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345); // Port du serveur
  addr.sin_addr.s_addr = inet_addr("192.168.1.88"); // Adresse IP du serveur (localhost)
  int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
  if(res==-1) { perror("connect"); exit(1); }
  //write(sock, "Hello", 6);

  char arg_dev[256];
  printf("entrer le chemin de fichier à envoyer un message: ex '/dev/my_led'\n");
  scanf("%s",arg_dev);
  write(sock, arg_dev, strlen(arg_dev));
  
  char buf[256];
  do{
    printf("entrer une commande à envoyer à %s (quit pour quitter): \n", arg_dev);
    scanf("%s",buf);
    write(sock, buf, strlen(buf));
    //read(sock, buf, 6);
    //printf("reçu: %s\n", buf);
  }while(strcmp(buf,"quit") != 0);

  close(sock);
  return EXIT_SUCCESS;

}
