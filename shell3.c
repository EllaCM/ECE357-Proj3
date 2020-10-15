/*draft branch*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

void printCommand(char **command, int cmd_length);

int isRedir(char arg);

void clearCommand(char **command, int cmd_length);

int main(int argc, char *argv[]){
	struct timeval realStart, realEnd, real;
	size_t length = 4096;
	char line[length];
	char *lineptr = line;
	int bytes_read;
	pid_t w;
	struct rusage ru;
	int return_val;
	while(bytes_read = getline(&lineptr, &length, stdin)!=-1)
	{
		gettimeofday(&realStart, NULL);
		if(!bytes_read || *lineptr=='#' || *lineptr == 10){printf("No Command\n"); continue;}
		int cmd_length = 0;
		int redirIndex = 0; //Index of token at which redirection starts
		char **cmd = malloc(sizeof(char*));
		char **cmdArgs = malloc(sizeof(char*));
		char *token;
		size_t currentSize = 0;
		int currentToken = 0;
		while((token=strtok(lineptr, " "))!=NULL){
			int tokenIndex = 0;
			while(token[tokenIndex] && token[tokenIndex] != 10)
			{
				if(isRedir(token[tokenIndex]) && !redirIndex)
				{
					redirIndex = cmd_length;
					printf("Index where redirection starts: %d\n", redirIndex);
				}
				tokenIndex++;
			}
			char *tokenCpy = malloc(tokenIndex);
			for(int i = 0; i < tokenIndex; i++)
			{
				tokenCpy[i] = token[i];
			}
			cmd[cmd_length] = tokenCpy;
			cmd_length++;
			cmd = realloc(cmd, (cmd_length+1)*sizeof(char*));
			cmd[cmd_length] = NULL; //empty out any strings from last time
			if(lineptr){lineptr = NULL;}
		}
		printCommand(cmd, cmd_length);
		int argLength = 0;
		if(redirIndex)
		{
			argLength = redirIndex;
			cmdArgs = realloc(cmdArgs, argLength*sizeof(char*)); //Create vector of arguments without redirection
			for(int i = 0; i<redirIndex;i++)
			{
				cmdArgs[i] = cmd[i];
			}
			printCommand(cmdArgs, redirIndex);
		}
		else
		{
			argLength = cmd_length+1;
			cmdArgs = realloc(cmdArgs, argLength*sizeof(char*));
			cmdArgs = cmd;
			printCommand(cmdArgs, cmd_length);
		}
		//built-in commands
		if(!strcmp(cmd[0], "cd"))
		{
			if(cmd_length > 2)
			{
				fprintf(stderr, "Error: cd command only accepts one argument\n");
			}
			else
			{
				if(cmd_length ==1)
				{
					cmd[1] = getenv("HOME");
					printf("Home directory: %s\n", cmd[1]);
				}
				if(chdir(cmd[1])==-1){fprintf(stderr,"Cannot change the directory: %s.\n",strerror(errno));}
			}
		}
		else if(!strcmp(cmd[0],"pwd"))
		{
			if(cmd_length > 1)
			{
				fprintf(stderr, "Error: pwd command accepts no arguments\n");
			}
			else
			{
				char *buf = malloc(1024);
				if(getcwd(buf,1024)==NULL){fprintf(stderr,"Cannot get current directory: %s.\n",strerror(errno));}
				else
				{
					fprintf(stdout,"%s\n",buf);
				}
			}
		}
		else if(!strcmp(cmd[0],"exit"))
		{
			if(cmd_length > 2)
			{
				fprintf(stderr, "Error: exit command only accepts one argument\n");
			}
			else
			{
				if(cmd_length == 1)
				{
					return 0;
				}
				else
				{
					/*reason for using strtol instead of atoi:
					atoi cannot distinguish a zero return, an invalid return, and an out_of_range return*/
					char **endptr = malloc(sizeof(cmd));
					errno = 0;
					long status = strtol(cmd[1], endptr, 10);
					if(**endptr)
					{
						fprintf(stderr, "Error: specified status value is not a number\n");
					}
					else if(errno == ERANGE)
					{
						fprintf(stderr, "Error: Specified status value is out of range\n");
					}
					else
					{
						free(cmd);
						return status;
					}
					free(endptr);
				}
			}
		}
		else //non built-in commands
		{
			if(fork() == 0)
			{
				int fd;
				for(int i = redirIndex; i < cmd_length; i++)
				{
					char *rdrTkn = cmd[i];
					char *fileName = NULL;
					switch(rdrTkn[0])
					{
						case '<':
							fileName = rdrTkn + 1;
							if((fd = open(fileName, O_RDONLY, 0666)) == -1)
							{
								fprintf(stderr, "Error: Could not open file for reading\n");
							}
							if(dup2(fd, 0)<0){
									fprintf(stderr, "Could not duplicate the fd\n");
							}
							close(fd);
							break;
						case '>':
							if(rdrTkn[1] == '>')
							{
								fileName = rdrTkn + 2;
								if((fd = open(fileName, O_WRONLY|O_CREAT|O_APPEND, 0666)) == -1)
								{
									fprintf(stderr, "Error: Could not open or create file for writing in append mode\n");
								}
									
							}
							else
							{
								fileName = rdrTkn + 1;
								if((fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1)
								{
									fprintf(stderr, "Error: Could not open or create file for writing in truncation mode\n");
								}
							}
							printf("File name: %s\n", fileName);
							printf("fd: %d\n", fd);
							if(dup2(fd, 1) < 0)
							{
								fprintf(stderr, "Could not redirect stdout to specified file: %s\n", strerror(errno));
							}
							close(fd);
							break;
						case '2':
							if(rdrTkn[2]=='>')
							{
								fileName = rdrTkn +3;
								if((fd = open(fileName, O_WRONLY|O_CREAT|O_APPEND, 0666)) == -1)
								{
									fprintf(stderr, "Error: Could not open or create file for writing in append mode\n");
								}
								
							}else{
								fileName = rdrTkn +2;
								if((fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1)
								{
									fprintf(stderr, "Error: Could not open or create file for writing in truncation mode\n");
								}
							}
							if(dup2(fd, 2) < 0)
							{
								fprintf(stderr, "Could not redirect stderr to specified file: %s\n", strerror(errno));
							}
							close(fd);
							break;
					}
				}
				if(execvp(cmdArgs[0],cmdArgs)<0){
					fprintf(stderr, "Failed to execute: %s\n", strerror(errno));
					exit(127);
				}
				//return 0;		
			}
			
			// wait somewhere here
			w = wait3(&return_val, 0, &ru);
			if(w==-1)	fprintf(stderr, "Wait failed.\n");
			else{
				if(return_val==0){
					fprintf(stderr, "Child process %d exited normalley\n", w);
				}else{
					if(WIFSIGNALED(return_val))
						fprintf(stderr, "Child process %d exited with signal%d\n", w, WTERMSIG(return_val));
					else
						fprintf(stderr, "Child process %d exited with non-zero return value %d.\n", w, WEXITSTATUS(return_val));
				}
			}
			gettimeofday(&realEnd, NULL);
			timersub(&realEnd, &realStart, &real);
			fprintf(stdout, "Real: %ld.%.6lds User: %ld.%.6lds Sys: %ld.%.6lds\n", real.tv_sec, real.tv_usec, ru.ru_utime.tv_sec, ru.ru_utime.tv_usec,
				ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
		}
		clearCommand(cmdArgs, argLength);
		clearCommand(cmd, cmd_length+1);
		free(cmd);
	}
	return 0;
}

void clearCommand(char **command, int cmd_length)
{
	for(int i = 0; i<cmd_length;i++)
	{
		command[i] = NULL;
	}
}

void printCommand(char **command, int cmd_length)
{
	printf("Command is: ");
	for(int i = 0; i < cmd_length; i++)
	{
		if(i < cmd_length - 1)
			printf("%s ", command[i]);
		else
			printf("%s", command[i]);
	}
	printf("\n");
}

int isRedir(char arg)
{
	if(arg == '<' || arg == '>')
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
