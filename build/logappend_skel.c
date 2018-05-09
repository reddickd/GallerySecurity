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

#define MAX_NUM 1073741823
//parse all of the information about the command into variables, store that 
//into text log file
//figure out how to manage making sure someone first enters gallery before any rooms,
//and how to make sure someone leaves 
//when someone leaves a room they are in the gallery ie a roomID is specified
//get each line of the file, parse it in the correct style, check if name is same as 
//command line, next line if not...otherwise update ur shit



int file_write(char* timestamp, char* name, char* logpath, int isEmp, int isArr,char* room, char** previous_data,int num_lines){
	FILE *n_log;
	n_log = fopen(logpath, "w+");
	if(n_log==NULL){
		printf("invalid\n");
		exit(255);
	}

	if(previous_data!=NULL){
		int i;
		for(i = 0;i<num_lines;i++){
			fwrite(previous_data[i],1,strlen(previous_data[i]),n_log);
			fwrite("\n",1,1,n_log);
		}
		//fwrite("\n",1,1,n_log);
	}
	
	int testing = 0;
  	if(testing){
  		fwrite("H|e|l|l|o\n",1,10,n_log);
  		fwrite("D|a|v|i|d\n",1,10,n_log);
  	}else{
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
    if(isArr==0&&room !=NULL){
    	fwrite(room,1,1,n_log);
	}else if(isArr == 1&& room!=NULL) {
		fwrite(room,1,strlen(room),n_log);
	}else if(isArr == 0&&room ==NULL){
		fwrite("-",1,1,n_log);
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
	}
    fclose(n_log);
	
    return 0;
}

//how to prevent " " in name...just accepts it as new argument


int is_valid_name(char* name){
	int j;
	for(j=0;j<strlen(name);j++){
		if(isalpha(name[j])==0){
			printf("invalid\n");
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
	//int token_spot;
	int fsize;
	int new_name = 1;
	int already_specified_emp_gue = 0;
	int already_specified_arr_dep = 0;
	char *logpath = NULL;
	char *name;
	char *timestamp;
	char *room=NULL;
	int room_int,time_int;
	unsigned char token[16];
	unsigned char iv[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//unsigned char tag[16];
	unsigned char* out_data;
	unsigned char* i_msg;
	int out_len1 = 0;
	int out_len2 = 0;
	int k;

	EVP_CIPHER_CTX *ctx;
 
  	


  //pick up the switches
  while ((opt = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1) {
    switch(opt) {
      case 'B':
        printf("unimplemented");

      case 'T':
        //timestamp
        //timestamp = malloc(sizeof(char*)*strlen(argv[arg_count]));
        timestamp = argv[arg_count];
        time_int = atoi(timestamp);
        if(time_int<=0||time_int>MAX_NUM){
        	printf("invalid\n");
        	exit(255);
        }
        arg_count += 2;
        break;

      case 'K':
        //secret token
        // token = malloc(sizeof(char*)*strlen(argv[arg_count]));
        // token = argv[arg_count];
      	if(strlen(argv[arg_count])>16){
      		for(k = 0;k<16;k++){
      			token[k] = argv[arg_count][k];
      		}
      	}else{
      		for(k = 0; k < strlen(argv[arg_count]);k++){
				if(argv[arg_count][k] == '\0'){
					token[k] = '\x20';
				}else{
					token[k] = argv[arg_count][k];
				}
			}
			for(k = strlen(argv[arg_count]); k < 16;k++){
				token[k] = '\x20';
			}
			// line[strlen(line)+1] = '\x20';
			// strcat(pad,line);
			token[16] = '\0';
      	}
      	
      	//token[17] = '\n';
      	//token_spot = arg_count;
        arg_count += 2;
        break;
        //should always be next argument after K

      case 'A':
        //arrival
      	if(already_specified_arr_dep){
      		printf("invalid\n");
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
      		printf("invalid\n");
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
      		printf("invalid\n");
      		exit(255);
      	}else{
	        //name = malloc(sizeof(char*)*strlen(argv[arg_count]));
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
      		printf("invalid\n");
      		exit(255);
      	}else{
        	//name = malloc(sizeof(char*)*strlen(argv[arg_count]));
        	name = argv[arg_count];
        	is_valid_name(name);
        	arg_count += 2;
        	isEmp = 0;
        	already_specified_emp_gue = 1;
    	}
        break;

      case 'R':
        //room ID
  		//room = malloc(strlen(argv[arg_count]));
  		room = argv[arg_count];
  		
  		while(*room && *room=='0'){
  			room++;
  		}
  		room_int = atoi(room);
  		if(room_int<0||room_int>MAX_NUM){
  			printf("invalid\n");
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

    //ssize_t read;
    //size_t len;
    int i = 0;
    int i_len;
    //char *line = NULL;
    char *prev_name,*prev_timestamp,*prev_emp_gue,*prev_room,*prev_arr_dep;
    char *recent_room,*recent_arr_dep,*recent_name;//,*recent_timestamp, *recent_emp,*/ 

    
    
    //opens old log and goes through line by line
    if(access(logpath,F_OK)!=-1){ //if file exists

    	//decrypt old log here

    	
    	log = fopen(logpath,"r");

    	fseek(log,0L,SEEK_END);
    	fsize = ftell(log);
    	fseek(log,0L,SEEK_SET);

    	unsigned char buffer[fsize];
    	memset(buffer, 0,fsize);
    	i_msg = malloc(fsize);
    	out_data = malloc(fsize*2);
    	i_len = fread(buffer,1,fsize,log);
    	buffer[fsize+1] = '\0';
    	fclose(log);

    	

    	i_msg[i_len] = '\0';

    	ctx = EVP_CIPHER_CTX_new();
    	if(!EVP_DecryptInit(ctx,EVP_aes_256_cbc(),token,iv)){
    		printf("invalid\n");
    		exit(255);
    	}
    	EVP_DecryptUpdate(ctx, out_data,&out_len1,buffer,fsize);

    	//if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,16,tag)){
    	// 	printf("invalid decrpyt");
    	// 	exit(255);
    	// }

    	EVP_DecryptFinal(ctx, out_data + out_len1,&out_len2);

    	out_data[out_len1+out_len2] = '\0';


    	char *copy_data = malloc(strlen((char*)out_data));
    	strcpy(copy_data,(char*)out_data);
    	//printf("%s",out_data);

    	//int total_len = out_len1+out_len2;
    	free(i_msg);
    	
    	int num_lines = 0;
    	int max_len = 0;
    	// int 
    	//log = fopen(logpath,"w");
    	
    	char* tok;
    	int a = 0;

    	tok = strtok((char*)out_data,"\n");
    	if(tok!=NULL&&strlen(tok)>max_len){
    			max_len = strlen(tok);
    		}
    	//temp = tok;
    	//strcpy(new_array[a],tok);
    	a++;
    	while(tok!=NULL){
    		tok = strtok(NULL,"\n");
    		if(tok!=NULL&&strlen(tok)>max_len){
    			max_len = strlen(tok);
    		}
    		if(tok == NULL){
    			num_lines = a;
    			break;
    		}
    		//strcpy(new_array[a],tok);
    		a++;
    	}
    	free(out_data);
    	//parse log here
    	char new_array[num_lines][max_len+1];
    	a = 0;
    	tok = strtok(copy_data,"\n");
    	strcpy(new_array[a],tok);
    	a++;
    	while(tok!=NULL){
    		tok = strtok(NULL,"\n");
    		if(tok!=NULL){
    			strcpy(new_array[a],tok);
    		}
    		a++;
    	}

    	//trims each string
    	for(a = 0; a < num_lines;a++){
    		int s = 0;	     		
    		for(s = 0;s<strlen(new_array[a]);s++){
    			if('\0'!=new_array[a][s]){
    				new_array[a][s] = new_array[a][s];
    			}else{
    				break;
    			}
    		}
    	}
    	
    	char *copy_array[num_lines];
    	for(a = 0;a<num_lines;a++){
    		copy_array[a] = strdup(new_array[a]);
    	}


    	//while((read = getline(&line,&len,log))!=-1){
    	a = 0; 
    	//while(strcmp(new_array[a],"")!=0){
    		//tok = strtok(line,"|");
    	//tok = strtok(new_array[a],"|");

    	//prev_timestamp = malloc(sizeof(char*)*strlen(tok));

    	for(a = 0; a < num_lines;a++){
	    	tok = strtok(new_array[a],"|");
	    	prev_timestamp = malloc(sizeof(char*)*strlen(tok));
			prev_timestamp = tok;
			
			i++;
			while(tok != NULL){

				tok = strtok(NULL,"|");
				if(tok!= NULL&&strcmp("|",tok)!=0&&i==1){
					//prev_name = malloc(sizeof(char*)*(strlen(tok)));
					prev_name = tok;
				}else if(tok!= NULL&&strcmp("|",tok)!=0&&i==2){
					//prev_emp_gue = malloc(sizeof(char*)*(strlen(tok)));
					prev_emp_gue = tok;
				}else if(tok!= NULL&&strcmp("|",tok)!=0&&i==3){
					//prev_room = malloc(sizeof(char*)*(strlen(tok)));
					prev_room = tok;
				}else if(tok!= NULL&&strcmp("|",tok)!=0&&i==4){
					//prev_arr_dep = malloc(sizeof(char*)*(strlen(tok)));
					prev_arr_dep = tok;
				}
				
				i++;
			}//will save most recent data of the current name from the command
			i = 0;
			if(strcmp(prev_name,name)==0&&((strcmp(prev_emp_gue,"EM")==0&&isEmp==1)||(strcmp(prev_emp_gue,"GU")==0&&isEmp==0))){ 
				new_name = 0;

				recent_name = malloc(sizeof(char*)*(strlen(prev_name)));
				strcpy(recent_name,prev_name);
			
				// recent_emp = malloc(sizeof(char*)*(strlen(tok)));
				// recent_emp = prev_emp_gue;
			
				recent_room = malloc(sizeof(char*)*(strlen(prev_room)));
				strcpy(recent_room,prev_room);
			
				recent_arr_dep = malloc(sizeof(char*)*(strlen(prev_arr_dep)));
				strcpy(recent_arr_dep,prev_arr_dep);	
			}
		}

		//a++;
	//}
    	
    	fclose(log);

    	//saving the most recent line in log where the name and employment status matched
    	//in all of the recent_ variables
    	
    	
    	int prev_time_int = atoi(prev_timestamp);

    	if(prev_time_int >= time_int){
    		printf("invalid\n");
    		exit(255);
    	}
    	if(new_name == 1){
			if(isArr==1&&room==NULL){
				file_write(timestamp,name,logpath,isEmp,isArr,NULL,copy_array,num_lines);
			}else{
				printf("invalid\n");
				exit(255);
			}	
		}else if(isArr==1&&strcmp("DP",recent_arr_dep)!=0&&strcmp("-",recent_room)!=0){
			printf("invalid\n");
			exit(255);				
		}else if(room!=NULL&&isArr == 1&&strcmp(room,recent_room)==0&&strcmp("DP",recent_arr_dep)!=0){
			printf("invalid\n");
			exit(255);
		}else if(room == NULL && isArr == 1&& strcmp("-",recent_room)==0&&strcmp("AV",recent_arr_dep)==0){
			printf("invalid\n");
			exit(255);
		}else if(room!=NULL&&isArr == 0&&strcmp("DP",recent_arr_dep)==0&&strcmp(room,recent_room)!=0){
			printf("invalid\n");
			exit(255);
		}else if(room!=NULL&&isArr == 0&&strcmp(recent_room,"-")!=0&&strcmp("DP",recent_arr_dep)==0){
			printf("invalid\n");
			exit(255);
		}else if(room == NULL&&strcmp("DP",recent_arr_dep)==0&&strcmp("-",recent_room)==0&&isArr==0){
			printf("invalid\n");
			exit(255);
		}else if(room != NULL&&strcmp("DP\n",recent_arr_dep)==0&&strcmp("-",recent_room)==0){
			printf("invalid\n");
			exit(255);
		}else if(room!=NULL&&isArr == 0&&strcmp(recent_room,room)!=0&&strcmp("AV\n",recent_arr_dep)==0){
			printf("invalid\n");
			exit(255);
		}else if(room == NULL&&isArr == 0&&strcmp("-",recent_room)!=0&&strcmp("AV\n",recent_arr_dep)==0){
			printf("invalid\n");
			exit(255);
		}else if(room == NULL&&strcmp("-",recent_room)){
			//printf("leaving gallery");
			file_write(timestamp,name,logpath,isEmp,isArr,NULL,copy_array,num_lines);
		}else{
			file_write(timestamp,name,logpath,isEmp,isArr,room,copy_array,num_lines);
		}
  		
  		
  		FILE *fp = fopen(logpath,"r");
  		if(fp==NULL){
  			printf("invalid");
  			exit(255);
  		}
  		fseek(fp,0L,SEEK_END);
  		fsize = ftell(fp);
  		fseek(fp,0L,SEEK_SET);
  		i_msg = malloc(fsize);
  		out_data = malloc(fsize*2);
  		fread(i_msg,sizeof(char),fsize,fp);
  		fclose(fp);
  		
  		ctx = EVP_CIPHER_CTX_new();
    	EVP_EncryptInit(ctx, EVP_aes_256_cbc(),token,iv);
    	EVP_EncryptUpdate(ctx,out_data,&out_len1,i_msg,fsize);
    	EVP_EncryptFinal(ctx,out_data+out_len1,&out_len2);
    	out_data[out_len1+out_len2] = '\0';
    	int length = out_len1+out_len2;
    	//EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG,16,tag);

   		fp = fopen(logpath,"w+");
   		fwrite(out_data,1,length,fp);
   		//fwrite("\n",1,1,fp);
   		//fwrite(tag,1,16,fp);
   		fclose(fp);
		
    }else{//else if file doesnt exist aka first entry
		if(room!=NULL||time_int<0||isArr==0){//and room is not specified
	   		printf("invalid\n");
			exit(255);
		}else{
			
			//decrpyt new log


			file_write(timestamp,name,logpath,isEmp,isArr,NULL,NULL,0);

			FILE *fp = fopen(logpath,"r");
			if(fp==NULL){
				printf("invalid\n");
				exit(255);
			}
			fseek(fp,0L,SEEK_END);
			fsize = ftell(fp);
			fseek(fp,0L,SEEK_SET);
			i_msg = malloc(fsize);
			out_data = malloc(fsize*2);
			fread(i_msg,sizeof(char),fsize,fp);
			fclose(fp);
			
			ctx = EVP_CIPHER_CTX_new();
			EVP_EncryptInit(ctx, EVP_aes_256_cbc(),token,iv);
			EVP_EncryptUpdate(ctx,out_data,&out_len1,i_msg,fsize);
			EVP_EncryptFinal(ctx,out_data+out_len1,&out_len2);
			out_data[out_len1+out_len2] = '\0';
			int length = out_len1+out_len2;
			//EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG,16,tag);

			fp = fopen(logpath,"w+");
			fwrite(out_data,1,length,fp);
			//fwrite("\n",1,1,fp);
			//fwrite(tag,1,16,fp);
			fclose(fp);
		}
	}

	//free EVERYTHING i think...
	// if(recent_room!=NULL){
	// 	free(recent_room);
	// }
	// if(recent_arr_dep!=NULL){
	// 	free(recent_arr_dep);
	// }
	//free(recent_room);
	//free(recent_arr_dep);
    // free(room);
    // if(name!=NULL){
    // 	free(name);
    // }
    //free(name);
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
