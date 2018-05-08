#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>


//parse all of the information about the command into variables, store that 
//into text log file
//figure out how to manage making sure someone first enters gallery before any rooms,
//and how to make sure someone leaves 
//when someone leaves a room they are in the gallery ie a roomID is specified
//get each line of the file, parse it in the correct style, check if name is same as 
//command line, next line if not...otherwise update ur shit


int file_write(char* timestamp, char* name, char* logpath, int isEmp, int isArr,char* room){
	FILE *n_log;
	n_log = fopen(logpath, "a");
			
  
    fwrite(timestamp,1,strlen(timestamp),n_log);
    fwrite("|",1,1,n_log);
    fwrite(name,1,strlen(name),n_log);
    fwrite("|",1,1,n_log);
    if(isEmp == 1){
  	  fwrite("EM",1,2,n_log);
    }else{
  	  fwrite("GU",1,2,n_log);
    }
    fwrite("|",1,1,n_log);
    if(isArr==0){
    	fwrite("-",1,1,n_log);
	}else if(isArr == 1&& room!=NULL) {
		fwrite(room,1,strlen(room),n_log);
	}else if(isArr == 0&&room !=NULL){
		fwrite(room,1,strlen(room),n_log);
	}else{
		fwrite("-",1,1,n_log);
	}
    fwrite("|",1,1,n_log);
    if(isArr == 1){
  	  fwrite("AV",1,2,n_log);
    }else{
  	  fwrite("DP",1,2,n_log);
    }
    fwrite("\n",1,1,n_log);
    fclose(n_log);
    return 0;
}

//how to prevent " " in name...just accepts it as new argument


int is_valid_name(char* name){
	int j;
	for(j=0;j<strlen(name);j++){
		if(isalpha(name[j])==0){
			printf("invalid name");
			exit(255);
		}
	}
	return 0;
}

int parse_cmdline(int argc, char *argv[]) {

	FILE *log;
	int opt = -1;
	int is_good = -1;
	int arg_count = 2;
	int isEmp = 0;
	int isArr = 0;
	int new_name = 1;
	int already_specified_emp_gue = 0;
	int already_specified_arr_dep = 0;
	char *logpath = NULL;
	char *name;
	char *timestamp;
	char *room=NULL;
	int room_int,time_int;
 
  	


  //pick up the switches
  while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1) {
    switch(opt) {
      case 'B':
        printf("unimplemented");

      case 'T':
        //timestamp
        timestamp = malloc(sizeof(char*)*strlen(argv[arg_count]));
        timestamp = argv[arg_count];
        time_int = atoi(timestamp);
        if(time_int<=0){
        	printf("invalid,time cant be less than 0");
        	exit(255);
        }
        arg_count += 2;
        break;

      case 'K':
        //secret token
        // token = malloc(sizeof(char*)*strlen(argv[arg_count]));
        // token = argv[arg_count];
        arg_count += 2;
        break;
        //should always be next argument after K

      case 'A':
        //arrival
      	if(already_specified_arr_dep){
      		printf("invalid,duplicate command");
      		exit(255);
      	}else{
	        arg_count++;
	        isArr = 1;
	        already_specified_arr_dep = 1;
	    }
        break;

      case 'L':
        //departure
      if(already_specified_arr_dep){
      		printf("invalid,duplicate command");
      		exit(255);
      	}else{
        	arg_count++;
        	isArr = 0;
        	already_specified_arr_dep = 1;
        }
        break;

      case 'E':
        //employee name
      	if(already_specified_emp_gue){
      		printf("invalid,duplicate command");
      		exit(255);
      	}else{
	        name = malloc(sizeof(char*)*strlen(argv[arg_count]));
	        name = argv[arg_count];
	        is_valid_name(name);
	        arg_count += 2;
	        isEmp = 1;
	        already_specified_emp_gue = 1;
    	}
        break;

      case 'G':
        //guest name
      if(already_specified_emp_gue){
      		printf("invalid,duplicate command");
      		exit(255);
      	}else{
        	name = malloc(sizeof(char*)*strlen(argv[arg_count]));
        	name = argv[arg_count];
        	is_valid_name(name);
        	arg_count += 2;
        	isEmp = 0;
        	already_specified_emp_gue = 1;
    	}
        break;

      case 'R':
        //room ID
  		room = malloc(strlen(argv[arg_count]));
  		room = argv[arg_count];
  		room_int = atoi(room);
  		if(room_int<0){
  			printf("invalid, room must be positive");
  			exit(255);
  		}
		arg_count += 2;

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
    ssize_t read;
    size_t len;
    int i = 0;
    char *line = NULL;
    char *prev_name,*prev_timestamp,*prev_emp_gue,*prev_room,*prev_arr_dep;
    char *recent_room,*recent_arr_dep;/**recent_name,*recent_timestamp, *recent_emp,*/ 

    char *tok;
    //decrypt old log here
    //opens old log and goes through line by line
    if(access(logpath,F_OK)!=-1){ //if file exists
    	//parse log here
    	log = fopen(logpath,"r");
    	while((read = getline(&line,&len,log))!=-1){
    		
    		tok = strtok(line,"|");
    		prev_timestamp = malloc(sizeof(char*)*strlen(tok));
    		prev_timestamp = tok;
    		
    		i++;
    		while(tok != NULL){

    			tok = strtok(NULL,"|");
    			if(tok!= NULL&&strcmp("|",tok)!=0&&i==1){
    				prev_name = malloc(sizeof(char*)*(strlen(tok)));
    				prev_name = tok;
    			}else if(tok!= NULL&&strcmp("|",tok)!=0&&i==2){
    				prev_emp_gue = malloc(sizeof(char*)*(strlen(tok)));
    				prev_emp_gue = tok;
    			}else if(tok!= NULL&&strcmp("|",tok)!=0&&i==3){
    				prev_room = malloc(sizeof(char*)*(strlen(tok)));
    				prev_room = tok;
    			}else if(tok!= NULL&&strcmp("|",tok)!=0&&i==4){
    				prev_arr_dep = malloc(sizeof(char*)*(strlen(tok)));
    				prev_arr_dep = tok;
    			}
    			
    			i++;
    		}//will save most recent data of the current name from the command
    		i = 0;
    		if(strcmp(prev_name,name)==0&&((strcmp(prev_emp_gue,"EM")==0&&isEmp==1)||(strcmp(prev_emp_gue,"GU")==0&&isEmp==0))){ 
    				new_name = 0;

    				// recent_name = malloc(sizeof(char*)*(strlen(tok)));
    				// recent_name = prev_name;
    			
    				// recent_emp = malloc(sizeof(char*)*(strlen(tok)));
    				// recent_emp = prev_emp_gue;
    			
    				recent_room = malloc(sizeof(char*)*(strlen(prev_room)));
    				strcpy(recent_room,prev_room);
    			
    				recent_arr_dep = malloc(sizeof(char*)*(strlen(prev_arr_dep)));
    				strcpy(recent_arr_dep,prev_arr_dep);	
    		}

    	}
    	
    	fclose(log);

    	//saving the most recent line in log where the name and employment status matched
    	//in all of the recent_ variables
    	
    	
    	int prev_time_int = atoi(prev_timestamp);

    	if(prev_time_int >= time_int){
    		printf("invalid, time doesnt go backwards");
    		exit(255);
    	}
    	if(new_name == 1){
			if(isArr==1&&room==NULL){
				file_write(timestamp,name,logpath,isEmp,isArr,NULL);
			}else{
				printf("invalid, new guest needs to enter gallery first");
				exit(255);
			}	
		}else if(isArr==1&&strcmp("-",recent_room)!=0){
			printf("invalid, need to leave room first");
			exit(255);				
		}else if(room!=NULL&&isArr == 1&&strcmp(room,recent_room)==0){
			printf("cant enter a room twice");
			exit(255);
		}else if(room == NULL && isArr == 1&& strcmp("-",recent_room)==0){
			printf("cant enter gallery twice");
			exit(255);
		}else if(room!=NULL&&isArr == 0&&strcmp(recent_room,room)!=0){
			printf("have to leave the room you are in, not the leaving gallery check");
			exit(255);
		}else if(room == NULL&&strcmp("-",recent_room)){
			printf("leaving gallery");
		}else if(room!=NULL&&isArr == 0&&strcmp("DP",recent_arr_dep)==0&&strcmp(room,recent_room)!=0){
			printf("cant leave room twice unless leaving gallery");
			exit(255);
		}else{
			file_write(timestamp,name,logpath,isEmp,isArr,room);
		}
  
    	
    	//encrpyt again

   
		
    }else{//else if file doesnt exist aka first entry
		if(room!=NULL||time_int<0||isArr==0){//and room is not specified
	   		printf("invalid, first guest needs to enter gallery");
			exit(255);
		}else{
			
			//decrpyt new log


			file_write(timestamp,name,logpath,isEmp,isArr,NULL);
		}
	}

	//free EVERYTHING i think...
    // free(room);
    // free(departure);
    // free(arrival);
    // free(token);
    // free(timestamp);
    //free prev_arr_dep
    //free prev_name
    //free prev_emp_gue
    //free prev room

	//encrypt here
    return is_good;
}


int main(int argc, char *argv[]) {

  int result;
  result = parse_cmdline(argc, argv);
  if(result!= -1){
  	printf("bad");
  }



  return 0;
}
