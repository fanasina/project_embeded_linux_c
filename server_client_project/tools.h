#ifndef __TOOLS_HEAD_SRV_CLI__
#define __TOOLS_HEAD_SRV_CLI__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE_BUF 256

/* list cmd destination */
#define MOD_LED 1
#define MOD_INT 2 /* button */
#define MOD_LCD 3
#define MOD_SENSOR 4
#define LOG 5


#define MAIN_SERV 6
#define COUNT_FILES 7

#define MAX_CONN 10

#define MAIN_HAND_MSG 8
#define CLIENT 9

#define SIZE_BUF 256
#define SIZE_PSEUDO 25


extern char *list_dev[64];


char* extract_field_char_btw_colon(char *input, int field_to_extract);

char* extract_field_from_to_btw_colon(char *input, int from_field_to_extract, int to_field_to_extract);


long int parse_field_char_to_lint(char *input, int field_to_extract);
 
char* date_now(void);
//void handle_message(char * message);
#endif
