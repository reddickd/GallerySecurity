#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>


//parse all of the information about the command into variables, store that 
//into text log file

int parse_cmdline(int argc, char *argv[]) {

	FILE *log;
	int opt = -1;
	int is_good = -1;
	int arg_count = 2;
	int isEmp = 0;
	char  *logpath = NULL;
	char *token;
	char *name;
	char *timestamp;
	char *arrival;
	char *room;
 
  //char a_l_status[1] = ' '; 


  //pick up the switches
  while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1) {
    switch(opt) {
      case 'B':
        printf("unimplemented");

      case 'T':
        //timestamp
        timestamp = malloc(sizeof(char*)*strlen(argv[arg_count]));
        timestamp = argv[arg_count];
        arg_count += 2;
        break;

      case 'K':
        //secret token
        token = malloc(sizeof(char*)*strlen(argv[arg_count]));
        token = argv[arg_count];
        arg_count += 2;
        break;
        //should always be next argument after K

      case 'A':
        //arrival
        break;

      case 'L':
        //departure
        break;

      case 'E':
        //employee name
        name = malloc(sizeof(char*)*strlen(argv[arg_count]));
        name = argv[arg_count];
        arg_count += 2;
        isEmp = 1;
        break;

      case 'G':
        //guest name
        name = malloc(sizeof(char*)*strlen(argv[arg_count]));
        name = argv[arg_count];
        arg_count += 2;
        break;

      case 'R':
        //room ID
  		//room = malloc(strlen(argv[arg_count]));
  		//room = argv[arg_count];
		// arg_count += 2;
		break;
      default:
        //unknown option, leave
        break;
    }

  }


  	
  //pick up the positional argument for log path
  if(optind < argc) {
    logpath = argv[optind];
  }


  log = fopen(logpath, "a");
  fwrite("T: ",1,3,log);
  fwrite(timestamp,1,strlen(timestamp),log);
  fwrite(" Token: ",1,8,log);
  fwrite(token,1,strlen(token),log);
  fwrite(" Name: ",1,7,log);
  fwrite(name,1,strlen(name),log);
  if(isEmp == 1){
  	fwrite(" E: ",1,4,log);
  }else{
  	fwrite(" G: ",1,4,log);
  }
  fclose(log);


  //free(token);
  //free(timestamp);
  return is_good;
}


int main(int argc, char *argv[]) {

  int result;
  result = parse_cmdline(argc, argv);




  return 0;
}
