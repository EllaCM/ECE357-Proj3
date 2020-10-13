#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
	FILE *in;
	if(argc>1){
		int open_fd = open(argv[1],O_RDONLY);
		in = fdopen(open_fd, "r");
	}else{
		/*report error*/
	}
	if(in==NULL){
		fprintf(stderr,"Unable to open file: %s.\n",strerror(errno));
	}
	int read_in = 0;
	size_t length = 0;
	char *line = NULL;
	int bytes_read;
	char **cmd;
	int cmd_length = 0;
	int return_val;
	while(bytes_read = getline(&line, &length, in)!=-1){
		if(bytes_read==0 || (*line)=='#')	continue;
		char *token;
		while((token=strtok(line, " "))!=NULL){
			cmd= malloc(BUFSIZ*sizeof(char));
			strcpy(cmd[cmd_length], token);
			cmd_length++;
		}
		int str_len = strlen(cmd[cmd_length-1]);
		cmd[cmd_length] = NULL;
		if(strcmp(cmd[0],"cd")==0){
			if(cmd[1]==NULL)	cmd[1]=getenv("HOME");
			if(chdir(cmd[1])==-1)	fprintf(stderr,"Cannot change the directory: %s.\n",strerror(errno));
			return_val = 0;
			free(cmd);
		}else if(strcmp(cmd[0],"pwd")==0){
			char *buf = malloc(BUFSIZ);
			
			if(getcwd(buf,BUFSIZ)==NULL)	fprintf(stderr,"Cannot get current directory: %s.\n",strerror(errno));
			else{
				fprintf(stdout,"%s\n",buf);
			}
			return_val = 0;
			free(cmd);
		}else if(strcmp(cmd[0],"exit")==0){
			if(cmd[1]==NULL)	return_val = 0;
			else{
				/*reason for using strtol instead of atoi:
				atoi cannot distinguish a zero return, an invalid return, and an out_of_range return*/
				char *end;
				int lnum = strtol(cmd[1], &end, 10);
				if(end==cmd[1]){
					fprintf(stderr, "Cannot convert string to number\n");
					return_val = EXIT_FAILURE;
				}
				if((lnum>INT_MAX || lnum<INT_MIN) && errno == ERANGE){
					fprintf(stderr, "Number out of range!");
					return_val = EXIT_FAILURE;
				}
				return_val = (int)lnum;
			}
			free(cmd);
		}else{
			/*fork and status check*/
			fprintf(stdout, "fork and status check\n");
			return_val = 0;
		}
	}
	return 0;

}
