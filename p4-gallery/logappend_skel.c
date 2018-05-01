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




int parse_cmdline(int argc, char *argv[]) {

  int opt = -1;
  int is_good = -1;


  //pick up the switches
  while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1) {
    switch(opt) {
      case 'B':
        //batch file
        break;

      case 'T':
        //timestamp
        break;

      case 'K':
        //secret token
        break;

      case 'A':
        //arrival
        break;

      case 'L':
        //departure
        break;

      case 'E':
        //employee name
        break;

      case 'G':
        //guest name
        break;

      case 'R':
        //room ID
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




  return is_good;
}


int main(int argc, char *argv[]) {

  int result;
  result = parse_cmdline(argc, argv);




  return 0;
}
