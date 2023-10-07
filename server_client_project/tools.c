




#include "tools.h" 

char *list_dev[64]={
  "",
 "/dev/project_leds",
 "/dev/project_int_b_dev",
 "/dev/project_lcd_i2c1",
 "/dev/project_dth11",
  "/opt/log_temp",
};

char* date_now(void){
    time_t timestamp = time( NULL );
    struct tm * pTime = localtime( & timestamp );

    char buffer[SIZE_BUF];
    //strftime( buffer, SIZE_BUF, "%d/%m/%Y|%H:%M:%S", pTime );
    strftime( buffer, SIZE_BUF, "%Y%m%d|%H:%M:%S", pTime );
    char *retdate=malloc(strlen(buffer));
    strcpy(retdate, buffer);
    //printf( "Date and french time : %s\n", buffer );
    return retdate;
}

char* extract_field_char_btw_colon(char *input, int field_to_extract){
   size_t len = strlen(input);
   char *val=malloc(SIZE_BUF*2  +13);
   long int i, cur=0;
   int cnt_colon=0;
   for(i=0; i< len && cnt_colon <= field_to_extract; ++i){
      if(input[i] == ':' ){
        if(cnt_colon == field_to_extract){
          val[cur]='\0';
          return val;
        }
        ++cnt_colon;
        cur=0;
      }
      else {
        val[cur++] = input[i];
      }
   }
   val[cur]='\0';
   return val;
}

char* extract_field_from_to_btw_colon(char *input, int from_field_to_extract, int to_field_to_extract){
   size_t len = strlen(input);
   char *val=malloc(SIZE_BUF*2+13);
   long int i, cur=0;
   int cnt_colon=0;
   for(i=0; i< len && cnt_colon <= to_field_to_extract + 1; ++i){
      if(input[i] == ':' ){
        ++cnt_colon;
        if(cnt_colon == to_field_to_extract + 1 ){
          val[cur]='\0';
          return val;
        }
        else if(cnt_colon == from_field_to_extract){
          cur=0;
        }
        else{
          val[cur++] = input[i];
        }
      }
      else {
        val[cur++] = input[i];
      }
   }
   val[cur]='\0';
   return val;
}


long int parse_field_char_to_lint(char *input, int field_to_extract){
   size_t len = strlen(input);
   long int val = 0, i;
   int cnt_colon=0;
   for(i=0; i< len && cnt_colon <= field_to_extract; ++i){
      if(input[i] == ':' ){
        if(cnt_colon == field_to_extract)
          return val;
        ++cnt_colon;
        val=0;
      }
      if(input[i] >='0' && input[i]<='9'){
        val *= 10;
        val += (input[i]-'0');
      }
   }
   return val;
}
 

/*
void handle_message(char * message){
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


}
*/
