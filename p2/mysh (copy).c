/*
 * Author: Aashish Thite(thite@wisc.edu)
 *
 */
 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#define INP_SIZE 520
int interactive = 0;
int redirected=0;
const char* my_prompt; 
void print_error_message()
{
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}
void print_debug_message(char* msg)
{
	write(STDOUT_FILENO,msg,strlen(msg));
}
void print_prompt()
{
		write(STDOUT_FILENO,my_prompt,strlen(my_prompt));
}

void print_command(char* input)
{
	if(!interactive)
	{
		write(STDOUT_FILENO, input, strlen(input));
	}
}

int containsSemiColon(const char* line)
{
	int i=0;
	while(1)
	{
		if(line[i] == ';')
			return 1;
		if(line[i]=='\0')
			return 0;
		++i; 
	}

}

int containsPlus(char* line)
{
	int i=0;
	while(1)
	{
		if(line[i] == '+')
			return 1;
		if(line[i]=='\0')
			return 0;
		++i; 
	}
}

int split_line(char *line, char *words[]) 
{	
	char *input = strdup(line);
	int count = 0;
	while (1) 
	{
		while (isspace(*input)) input++;
		if (*input == '\0') 
			return count;
		words[count++] = input;
		while (!isspace(*input) && *input != '\0') 
			input++;
		if (*input == '\0') 
			return count;
		*input++ = '\0';
	 }
}
int handleCommandSequential(char* line)
{

	char* words[512];
	int word_count = split_line(line,words);
	if(word_count == 0) return 0;
	
	if(strcmp(words[0],"cd")==0)//cd
	{
		int retval = 0;
		if(word_count == 1) retval = chdir(getenv("HOME"));
		else retval = chdir(words[1]);
		if(retval != 0)	return -1;//error
		return 0;		
	}
	if(strcmp(words[0],"pwd")==0)
	{
		if(word_count !=1) return -1;
		char path[512];
		getcwd(path,sizeof(path));
		write(STDOUT_FILENO,path,strlen(path));
		write(STDOUT_FILENO, "\n", 1);
		return 0;
	}
	if(strcmp(words[0],"quit")==0) exit(0);//quit	
	pid_t cpid = fork();
	if(cpid<0)
	{
		return -1;
	}
	if(cpid==0)
	{
		char* args[513];
		int i;
		for(i=0;i<word_count;++i)
		{
			args[i]=words[i];
		}
		args[i] = NULL;
		if(execvp(words[0],args) <0)
		{
			print_error_message();
		}
		exit(0);
	}

	return 0;
}

void decodeAndExecuteCommand(FILE* inFile)
{
	char input[INP_SIZE];
	print_prompt();
	while(fgets(input,INP_SIZE,inFile)!=NULL)
	{
		print_command(input);
		redirected = 0;
		//Check for extra long input
		if(strlen(input)>512)
		{
			while(fgets(input,INP_SIZE,inFile)!=NULL)
				print_command(input);
			print_error_message();
			goto endofwhile;		
		}
		//check if input has to be executed in sequential or parallel
		int isSequential = containsSemiColon(input);
		int isParallel = containsPlus(input);
		if(isParallel && isSequential)
		{
			print_error_message();
			goto endofwhile;
		}
		//char* inputTokenizer = input;
		input[strlen(input)-1]='\0';
		char* token, *rest=NULL;
		char* ptr = input;	
		int bak=0, new;

		if(!isParallel)
		{
		
			while((token = strtok_r(ptr,";",&rest))!= NULL )
			{
				char* tokenizeRedir=token, *restRedir;
				int redirect =0;
				//Check for redirection
				if(strchr(token,'>')!=NULL)
				{ 
					tokenizeRedir= strtok_r(token,">",&restRedir);
					if(tokenizeRedir==NULL)//start with'>'
					{
						print_error_message();
						continue;
					}
					char* words_redir[512];
					int redirections = split_line(restRedir, words_redir);
					if(redirections!=1 || strchr(restRedir,'>'))//wrong redirection syntax
					{
						print_error_message();
						break;
					}
					redirect =1;
					//redirect to file here
					fflush(stdout);
					bak =dup(1);

					//close(STDOUT_FILENO);
					new = open(words_redir[0],O_CREAT | O_TRUNC |O_RDWR, 0777);
					if(new==-1)
					{
						print_error_message();
						//print_debug_message("cannot open file\n");
						break;
					}
					dup2(new,1);
					close(new);

				}
				int retval = handleCommandSequential(tokenizeRedir);
				if(retval == -1)
				{
					print_error_message();
					break;
				}
				ptr = rest;
				wait(NULL);	
				if(redirect)
				{
					//redirect back to stdout here
					fflush(stdout);
					dup2(bak,1);
					close(bak);
					continue;
				}
			}
	
		}
		if(isParallel)
		{
			
			while((token = strtok_r(ptr,"+",&rest))!= NULL )
			{
				char* tokenizeRedir=token, *restRedir;
				int redirect =0;
				//Check for redirection
				if(strchr(token,'>')!=NULL)
				{ 
					tokenizeRedir= strtok_r(token,">",&restRedir);
					if(tokenizeRedir==NULL)//start with'>'
					{
						print_error_message();
						break;//continue;
					}
					char* words_redir[512];
					int redirections = split_line(restRedir, words_redir);
					if(redirections!=1 || strchr(restRedir,'>'))//wrong redirection syntax
					{
						print_error_message();
						break;//continue;
					}
					redirect =1;
					//redirect to file here
					fflush(stdout);
					bak =dup(1);
					new = open(words_redir[0],O_CREAT | O_TRUNC |O_RDWR, 0777);
					if(new==-1)
					{
						print_error_message();
						break;
					}
					dup2(new,1);
					close(new);
				}
				int retval = handleCommandSequential(tokenizeRedir);
				if(retval == -1)
				{
					print_error_message();
					break;//continue;
				}
				ptr = rest;
				if(redirect)
				{
					//redirect back to stdout here
					fflush(stdout);
					dup2(bak,1);
					close(bak);
					continue;
				}
			}
		}
		while(wait(NULL)>0);

	endofwhile:
		print_prompt();
	}
}

int main(int argc, char** argv)
{
	FILE *inputFile;

	switch(argc)
	{
		case 1:
		{
			interactive = 1;
			inputFile = stdin;
			my_prompt = "cs537> ";
			break;
		}
		case 2:
		{
			inputFile = fopen(argv[1],"r");
			my_prompt = "";
			if(inputFile==NULL)
			{
				print_error_message();
				exit(1);
			}
			break;
		}
		default:
		{
			print_error_message();
			exit(1);
		}

	}
	decodeAndExecuteCommand(inputFile);	
	return(0);
}
