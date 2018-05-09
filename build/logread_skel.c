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

#define int_max 1073741823


char* concat(const char *s1, const char *s2);
int contains(const char *str, char **list, const int num);
static int myCompare(const void * str1, const void * str2);
void sort(char *list[], int num);
static int roomCompare(const void * a, const void * b);

struct room {
  int room_num;
  char **people;
  int num_people;
}room;

int main(int argc, char *argv[]) {

  FILE *fp;
  size_t line_length = 0;
  ssize_t len;
  int i, j, k, exists, opt, is_employee = 0, is_guest = 0;
  char  *logpath = NULL,
    // *TOKEN = NULL,
    *NAME = NULL,
    *line;
  extern char *optarg;
  int opt_S = 0, opt_R = 0, opt_I = 0, opt_T = 0;

  // decryption variables
  // unsigned char *ciphertext, *plaintext, key[16], iv[16];
  // int i_len, o_len1 = 0, o_len2 = 0;
  // EVP_CIPHER_CTX ctx;


  // parsing variables
  char *i_temp, *i_name, *i_role, *prev_action, *i_action;
  int prev_time = 0, i_time, i_room;

  // current state variables
  char **employee_names = NULL, **guest_names = NULL;
  int e_name_count = 0, g_name_count = 0;

  int num_rooms;
  struct room **all_rooms = NULL;


  // all rooms variables
  char *room_list = NULL, i_room_str[11];

  // init prev_action
  prev_action = NULL;

  while ((opt = getopt(argc, argv, "K:SRE:G:IT")) != -1) {
    // printf("%c\n", opt);
    switch(opt) {
      case 'K':
        // TOKEN = optarg;
        // TODO: check token is alphanumeric
        // TODO: check if decrypt works

        // create key

        // create iv
        // int i = 0;
        // for (i = 0; i < sizeof(iv); i++) {
        //   iv[i] = '\x00';
        // }

        break;

      case 'S':
        // print current state
        opt_S = 1;
        break;

      case 'R':
        // print all rooms entered by employee or guest
        opt_R = 1;
        break;

      case 'E':
        NAME = optarg;
        is_employee = 1;
        is_guest = 0;
        // printf("employee name: %s\n", NAME);
        break;

      case 'G':
        NAME = optarg;
        is_guest = 1;
        is_employee = 0;
        // printf("guest name: %s\n", NAME);
        break;

      case 'I':
        opt_I = 1;
        break;

      case 'T':
        opt_T = 1;
        break;
    }
  }

  // check for option cases
  if ((opt_S && opt_R) || (opt_S && opt_I) || (opt_S && opt_T) || (opt_R && opt_I) || (opt_R && opt_T) || ((opt_I && opt_T))) {
    // only one -S -R -I or -T and if both -R and -T
    printf("invalid\n");
    return 255;
  } else if (opt_I || opt_T) {
    // optional options
    printf("unimplemented\n");
    return 0;
  }

  if(optind < argc) {
    logpath = argv[optind];
  }
  // printf("logpath: %s\n", logpath);
  // printf("token: %s\n", TOKEN);
  // printf("name: %s\n", NAME);

  // check for -R used with employee or guest
  if (opt_R == 1 && (is_employee == 0 && is_guest == 0)) {
    printf("invalid\n");
    return 255;
  }

  // get and check the log file
  fp = fopen(logpath, "r");
  if (fp == NULL) {
    printf("invalid\n");
    return 255;
  }

  // // initial check for file not even containing tag
  // if (f_size < 16)
  //   return throw_inv();

  // // detach tag
  // i_len = f_size - 17;
  // j = 0;
  // for (i = 0; i < f_size; i++) {
  //   if (i < (i_len)) {
  //     // just the msg
  //     i_msg[i] = buffer[i];
  //   } else {
  //     // the tag
  //     tag[j] = buffer[i];
  //     j++;
  //   }
  // }

  // // Decrypt log file
  // ctx = EVP_CIPHER_CTX_new();
  // if (!EVP_DecryptInit(&ctx, EVP_aes_256_gcm(), key, iv)) {
  //   printf("integrity violation\n");
  //   return 255;
  // }
  // EVP_DecryptUpdate(&ctx, plaintext, &o_len1, ciphertext, i_len);

  // // check tag
  // if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag)) {
  //   printf("integrity violation\n");
  //   return 255;        
  // }
  // EVP_DecryptFinal(&ctx, plaintext + o_len1, &o_len2);
  // plaintext[o_len1+o_len2] = '\0';

  // go through the each line of the file
  while ((len = getline(&line, &line_length, fp)) != -1) {
    
    // end each line
    if (strchr(line, '\n')){
      line[len-1] = '\0';
    } else {
      line[len] = '\0';
    }
    
    // BEST FRIEND
    // printf("%s\n", line);

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

    if (opt_S == 1) {
      // current state line processing

      if (strcmp(i_role, "EM") == 0) {
        // get the names of employees

        if (e_name_count == 0) {
          // first name, add it
          employee_names = (char **) realloc(employee_names, (e_name_count + 1) * sizeof(char *));
          employee_names[e_name_count++] = strdup(i_name);

        } else {
          // check if name is already in
          if (!contains(i_name, employee_names, e_name_count)) {
            // add new name
            employee_names = (char **) realloc(employee_names, (e_name_count + 1) * sizeof(char *));
            employee_names[e_name_count++] = strdup(i_name);
          }

        }
      } else if (strcmp(i_role, "GU") == 0) {
        // get the names of guests

        if (g_name_count == 0) {
          // first name, add it
          guest_names = (char **) realloc(guest_names, (g_name_count + 1) * sizeof(char *));
          guest_names[g_name_count++] = strdup(i_name);

        } else {
          // check if name is already in
          if (!contains(i_name, guest_names, g_name_count)) {
            // add new name
            guest_names = (char **) realloc(guest_names, (g_name_count + 1) * sizeof(char *));
            guest_names[g_name_count++] = strdup(i_name);
          }

        }
      }

      // get room information
      if (i_room != -1 && (strcmp(i_action, "AV") == 0)) {
        // arriving in a room - add person to the room's list
        if (num_rooms == 0) {
          // first room - initialize array of room structs
          all_rooms = (struct room **) realloc(all_rooms, (num_rooms + 1) * sizeof(struct room *));
          all_rooms[num_rooms] = (struct room *) malloc(sizeof(struct room));

          // add room info
          all_rooms[num_rooms]->room_num = i_room;
          all_rooms[num_rooms]->num_people = 0;
          // add first person
          all_rooms[num_rooms]->people = NULL;
          all_rooms[num_rooms]->people = (char **) realloc(all_rooms[num_rooms]->people, (all_rooms[num_rooms]->num_people + 1) * sizeof(char *));
          all_rooms[num_rooms]->people[all_rooms[num_rooms]->num_people++] = strdup(i_name);

          num_rooms++;

        } else {
          // other occurances for rooms

          // check if room already exists
          exists = 0;
          for (i = 0; i < num_rooms; i++) {
            if (all_rooms[i]->room_num == i_room) {
              exists = 1;
              break;
            }
          }

          if (exists == 1) {
            // room exists, add person if not inside 
            // printf("%d exists\n", i_room);

            // check if person is already in the room's list
            if (!contains(i_name, all_rooms[i]->people, all_rooms[i]->num_people)) {
              // add new name
              all_rooms[i]->people = (char **) realloc(all_rooms[i]->people, (all_rooms[i]->num_people + 1) * sizeof(char *));
              all_rooms[i]->people[all_rooms[i]->num_people++] = strdup(i_name);
            }

          } else {
            // printf("creating new room for %d\n", i_room);
            // create new room
            all_rooms = (struct room **) realloc(all_rooms, (num_rooms + 1) * sizeof(struct room *));
            all_rooms[num_rooms] = (struct room *) malloc(sizeof(struct room));

            // add room info
            all_rooms[num_rooms]->room_num = i_room;
            all_rooms[num_rooms]->num_people = 0;

            // printf("adding %s\n", i_name);
            // add new people
            all_rooms[num_rooms]->people = NULL;
            all_rooms[num_rooms]->people = (char **) realloc(all_rooms[num_rooms]->people, (all_rooms[num_rooms]->num_people + 1) * sizeof(char *));
            all_rooms[num_rooms]->people[all_rooms[num_rooms]->num_people++] = strdup(i_name);
            // printf("added %s\n", i_name);

            num_rooms++;
            // printf("created new room for %d\n", i_room);
          }
        }
      } else if (i_room != -1 && (strcmp(i_action, "DP") == 0)) {
        // leaving a room - remove person from the room's list

        // find the specific room
        for (i = 0; i < num_rooms; i++) {
          if (all_rooms[i]->room_num == i_room) {
            break;
          }
        }
        if (i >= num_rooms) {
          // room was not found - log was wrong
          printf("integrity violation\n");
          return 255;
        } else {
            if (contains(i_name, all_rooms[i]->people, all_rooms[i]->num_people)) {
              // remove name if its contained

              for (j = 0; j < all_rooms[i]->num_people; j++) {
                // find matching name
                if (strcmp(i_name, all_rooms[i]->people[j]) == 0) {

                  // remove the name
                  free(all_rooms[i]->people[j]);
                  all_rooms[i]->people[j] = NULL;

                  // decrease num of people
                  all_rooms[i]->num_people -= 1;

                  // printf("removed %s from %d\n", i_name, i_room);
                  break;
                }
              }
              // [0, 1, 2, 3] -- 4 -> done
              // [0, NULL, 2, 3] -- 3 -> done
              // [0, NULL, 2] -- 2 -> done
              // [0, NULL] -- 1 -> done
              // [NULL -- 0] -> done
              // go through and shift down the ones after the removed name
              for (k = j; k < all_rooms[i]->num_people; k++) {
                if (k + 1 <= all_rooms[i]->num_people) {
                  // copy name from k + 1 to k
                  all_rooms[i]->people[k] = strdup(all_rooms[i]->people[k + 1]);
                  
                  if (k + 1 == all_rooms[i]->num_people) {
                    // remove free and NULL the the last entry after shifting
                    free(all_rooms[i]->people[k + 1]);
                    all_rooms[i]->people[k + 1] = NULL;
                  }
                }
              }

            }
        }

        // 

      }


    } else if (opt_R == 1) {
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
  fclose(fp);

    if (opt_S == 1) {
      // print out current state

      // sort the employee and guest names
      sort(employee_names, e_name_count);
      sort(guest_names, g_name_count);

      // print out employee names
      for (i = 0; i < e_name_count; i++) {
        if (i != 0) {
          printf(",");
        }
        printf("%s", employee_names[i]);
      }
      printf("\n");

      // print out guest names
      for (i = 0; i < g_name_count; i++) {
        if (i != 0) {
          printf(",");
        }
        printf("%s", guest_names[i]);
      }
      printf("\n");

      // free employee names
      if (employee_names != NULL) {
        for (i = 0; i < e_name_count; i++) {
          free(employee_names[i]);
        }
        free(employee_names);
      }

      // free guest names
      if (guest_names != NULL) {
        for (i = 0; i < g_name_count; i++) {
          free(guest_names[i]);
        }
        free(guest_names);
      }


      // sort all the rooms
      qsort(all_rooms, num_rooms, sizeof(struct room *), roomCompare);

      // print out list of rooms
      for(i = 0; i < num_rooms; i++) {
        if (all_rooms[i]->num_people > 0) {
          printf("%d: ", all_rooms[i]->room_num);

          // sort the list of people
          sort(all_rooms[i]->people, all_rooms[i]->num_people);

          // print out room's people
          for (j = 0; j < all_rooms[i]->num_people; j++) {
            if (j != 0) {
              printf(",");
            }
            printf("%s", all_rooms[i]->people[j]);
          }
          printf("\n");
        }
      }


      // free list of rooms and its people
      if (all_rooms != NULL) {
        for (i = 0; i < num_rooms; i++) {
          
          // loop through
          if (all_rooms[i]->people != NULL) {
            for (j = 0; j < all_rooms[i]->num_people; j++) {
              free(all_rooms[i]->people[j]);
            }
            free(all_rooms[i]->people);
          }

          free(all_rooms[i]);
          // i++;
        }
        free(all_rooms);
      }


    } else if (opt_R == 1) {
      // print out list of rooms for person
      if (room_list != NULL) {
        printf("%s\n", room_list);
        free(room_list);
      }
    }

    return 0;
}


char* concat(const char *s1, const char *s2) {
  const size_t len1 = strlen(s1);
  const size_t len2 = strlen(s2);

  char *result = malloc(len1 + len2 + 1);
  memcpy(result, s1, len1);
  memcpy(result + len1, s2, len2 + 1);
  return result;
}

int contains(const char *str, char **list, const int num) {
  int i;
  for (i = 0; i < num; i++) {
    if (strcmp(list[i], str) == 0)
      return 1;
  }
  return 0;
}

static int myCompare(const void * str1, const void * str2) {
  return strcmp(*(const char **) str1, *(const char **) str2);
}

void sort(char *list[], int num) {
  qsort(list, num, sizeof(const char *), myCompare);
}

static int roomCompare(const void *a, const void *b) {
  const struct room* roomA = *(struct room **)a;
  const struct room* roomB = *(struct room **)b;

  return (roomA->room_num < roomB->room_num) ? -1 : (roomA->room_num > roomB->room_num);
}
