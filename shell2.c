#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

void printCommand(char **command, int cmd_length);

int isRedir(char arg);

int main(int argc, char *argv[]){
	int read_in = 0;
	size_t length = 1024;
	char line[length];
	char *lineptr = line;
	int bytes_read;
	int return_val;
	while(bytes_read = getline(&lineptr, &length, stdin)!=-1)
	{
		if(bytes_read==0 || *lineptr=='#' || *lineptr == 10){printf("No Command\n"); continue;}
		int cmd_length = 0;
		int redirIndex = 0; //Index of token at which redirection starts
		char **cmd = malloc(sizeof(char*));
		char *token;
		char *lineParse = lineptr;
		size_t currentSize = 0;
		int currentToken = 0;
		while((token=strtok(lineParse, " "))!=NULL){
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
			if(lineParse){lineParse = NULL;}
		}
		printCommand(cmd, cmd_length);
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
			free(cmd);
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
			free(cmd);
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
					int status = strtol(cmd[1], endptr, 10);
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
				}
			}
			free(cmd);
		}
		else //non built-in commands
		{
			if(fork() == 0)
			{
				
			}
		}
		free(cmd);
	}
	return 0;

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