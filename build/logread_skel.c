#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>


// state_or_rooms = 0 for current state
// state_or_rooms = 1 for all rooms
char* concat(const char *s1, const char *s2);

int main(int argc, char *argv[]) {
  const int int_max = 1073741823;

  FILE *fp;
  size_t line_length = 0;
  ssize_t len;
  int i, opt, state_or_rooms = 0, is_employee = 0, is_guest = 0;
  char  *logpath = NULL,
    *TOKEN = NULL,
    *NAME = NULL,
    *line;
  extern char *optarg;

  // parsing variables
  char *i_temp, *i_name, *i_role, *prev_action, *i_action;
  int prev_time = 0, i_time, i_room;

  // all rooms variables
  char *room_list = NULL, i_room_str[11];

  // init prev_action
  prev_action = NULL;

  while ((opt = getopt(argc, argv, "K:SRE:G:IT")) != -1) {
    // printf("%c\n", opt);
    switch(opt) {
      case 'K':
        // TODO: get the token
        // check token is alphanumeric
        // check if decrypt works

        break;

      case 'S':
        // print current state
        state_or_rooms = 0;
        break;

      case 'R':
        // print all rooms entered by employee or guest
        state_or_rooms = 1;
        break;

      case 'E':
        NAME = optarg;
        is_employee = 1;
        // printf("employee name: %s\n", NAME);
        break;

      case 'G':
        NAME = optarg;
        is_guest = 1;
        // printf("guest name: %s\n", NAME);
        break;

      case 'I':
        printf("unimplemented\n"); 
        break;

      case 'T':
        printf("unimplemented\n");
        break;
    }
  }

  if(optind < argc) {
    logpath = argv[optind];
  }
  // printf("logpath: %s\n", logpath);

  // check for -R used with employee or guest
  if (state_or_rooms == 1 && (is_employee == 0 && is_guest == 0)) {
    printf("invalid\n");
    return 255;
  }

  // TODO: decrypt log file

  // get and check the log file
  fp = fopen(logpath, "r");
  if (fp == NULL) {
    printf("invalid\n");
    return 255;
  }

  // go through the each line of the file
  while ((len = getline(&line, &line_length, fp)) != -1) {
    
    // end each line
    if (strchr(line, '\n')){
      line[len-1] = '\0';
    } else {
      line[len] = '\0';
    }
    
    printf("%s\n", line);

    // parse line into respective variables
    i_temp = strtok(line, "|");
    i = 0;

    while (i_temp != NULL) {
      switch (i) {
        case 0:
          i_time = atoi(i_temp);

          // check for timestamp integrity violation
          if (i_time <= prev_time || i_time > int_max || i_time < 1) {
            printf("integrity violation\n");
            return 255;
          } else {
            prev_time = i_time;
          }
          break;
        case 1:
          // TODO: check for valid name
          i_name = i_temp;
          break;
        case 2:
          // check for valid role
          if ((strcmp(i_temp, "EM") != 0) && (strcmp(i_temp, "GU") != 0)) {
            printf("integrity violation\n");
            return 255;
          }
          i_role = i_temp;
          break;
        case 3:
          if (strcmp(i_temp, "-") == 0) {
            i_room = -1;
          } else {
            i_room = atoi(i_temp);
          }
          //check for valid room id
          if ((i_room < 0 && i_room != -1) || i_room > int_max) {
            printf("integrity violation\n");
            return 255;
          }
          break;
        case 4:
          // check for valid action
          if ((strcmp(i_temp, "AV") != 0) && (strcmp(i_temp, "DP") != 0)) {
            printf("integrity violation\n");
            return 255;
          }
          i_action = i_temp;
          break;
      }

      i_temp = strtok(NULL, "|");
      i++;
    }
    i = 0;

    if (state_or_rooms == 0) {
      // TODO: current state line processing


    } else if (state_or_rooms == 1) {
      // all rooms line processing
      // check for matching name, matching role, and action with a room
      if ((strcmp(NAME, i_name) == 0) && ( ((strcmp(i_role, "EM") == 0) && (is_employee == 1)) || ((strcmp(i_role, "GU") == 0) && (is_guest == 1)) ) && i_room >= 0) {

        // convert room # to string
        sprintf(i_room_str, "%d", i_room);

        // check for alternating arrival and departure & print
        if (prev_action == NULL) {
          // first arrival into room
          if (strcmp(i_action, "AV") == 0) {
            room_list = concat("", i_room_str);
          }
          prev_action = "AV";
        } else {
          if (strcmp(i_action, prev_action) == 0) {
            if (room_list != NULL) {
              free(room_list);
            }
            printf("integrity violation\n");
            return 255;
          } else {

            if (strcmp(i_action, "AV") == 0) {
              room_list = concat(room_list, ",");
              room_list = concat(room_list, i_room_str);
            }

            // update previous
            if (strcmp(i_action, "AV") == 0) {
              prev_action = "AV";
            } else if (strcmp(i_action, "DP") == 0) {
              prev_action = "DP";
            }
          }
        }
      }
    }

  }

    if (state_or_rooms == 0) {
      // TODO: print out current state

    } else if (state_or_rooms == 1) {
      // print out list of rooms
      if (room_list != NULL) {
        printf("%s\n", room_list);
        free(room_list);
      }
    }

}


char* concat(const char *s1, const char *s2) {
  const size_t len1 = strlen(s1);
  const size_t len2 = strlen(s2);

  char *result = malloc(len1 + len2 + 1);
  memcpy(result, s1, len1);
  memcpy(result + len1, s2, len2 + 1);
  return result;
}
