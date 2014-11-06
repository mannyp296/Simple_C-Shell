//Alex King and Manuel Perez
// 4/18/13
// Project 1, SISH (Simple Shell)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


#define TRUE 1

//Helper Functions
void shellPrompt(void);
void executeCommand(void);
void childProcess();
int getInput2();
void pipeline();
void outputFile();

//Variables
char command[1025];
int Argc = 0;
char *Argv[10];
char buffer[1025];
char input;
int background = 0;
int in = 0;
int out = 0;
int pipes = 0;
int test=0;
int prompt = 0;

int main(void) {
	signal(SIGINT, SIG_IGN);
	
	if(isatty(fileno(stdin))) {
		
		prompt = 1;
	}
	shellPrompt();
	while(TRUE) {
		if(getInput2()!=-1)
			executeCommand();
		shellPrompt();
	}
	return 0;
}

void executeCommand() {
	if(strcmp(Argv[0],"exit") == 0){
		exit(EXIT_SUCCESS);
	}
	if(strcmp(Argv[0],"cd") == 0) {
		if(Argv[1] == NULL) {
			chdir(getenv("HOME"));
		}
		else {
			if(chdir(Argv[1]) == -1) {
				perror("ERROR");
			}
		}
		return;
	}
	if(in || pipes) {
		pipeline();
		in = 0;
		out = 0;
		pipes = 0;
	}
	else if(out)
	{
		outputFile();
		out=0;
	}
	else
	{
		childProcess();
	}
}

void childProcess() {
	
	pid_t pid, pid2;
	pid = fork();
	char* tmp;
	int child_state;
	
	tmp = Argv[Argc-1];
	if(tmp[0] == '&') {
		background = 1;
		Argv[Argc-1] = NULL;
		Argc--;
	}
	else background=0;
	
	switch(pid) {
		case -1:
			perror("ERROR");
			exit(EXIT_FAILURE);
		case 0:
			//setpgid(0,0);
			if(execvp(*Argv,Argv) == -1) {
				perror("ERROR");
			}
			exit(EXIT_SUCCESS);
			break;
		default:
			if(background==1){
				background=0;
				break;
			}
			//printf("%i\n",background);
			do{
			pid2 = waitpid(pid,&child_state,WNOHANG);
			} while(pid2!=pid);
			//wait(0);
			break;
	}
}

void outputFile(){
	int i, fdOut, status;
	pid_t pid,pid2;
	char* args[10];
	char* myFile;
	for(i=0; i<Argc; i++)
	{
		if(strcmp(Argv[i],">")==0)
			break;
		args[i]=Argv[i];
	}
	myFile=Argv[i+1];
	
	pid=fork();
	if(pid==0)
	{
		fdOut = open(myFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
			dup2(fdOut, STDOUT_FILENO);
		if(execvp(args[0], args)==-1)
			perror("ERROR");
		exit(EXIT_SUCCESS);
	}
	else
	{
		do{
			pid2 = waitpid(pid,&status,WNOHANG);
		}while(pid2!=pid);
	}
}



void pipeline() {
	
	int i, j,k=0, status, x;
	int fd_old[2], fd_new[2];
	pid_t pid, pid2;
	char* args[10];
	int op;
	char *myFile;
	char* myFile2;
	
	
	for(i=0;i<pipes+1;i++) {
		op = 0;
		
		//Get command arguments for each pipe
		for(j=k; j<Argc;j++)
		{
			// printf("i: %i, j: %i, k: %i\n",i,j,k);
			if (strcmp(Argv[j],"|")==0)
			{
				if(j==0)
				{
					printf("ERROR: syntax error near unexpected token '|'\n");
					return;
				}
				else
				{
					for(x=0;x<j-k;x++) {
						if ((strcmp(Argv[k+x],"<")==0))
						{
							op=1;
							myFile=Argv[k+x+1];
							break;
						}
						args[x] = Argv[k+x];
					}
				}
				break;
			}
		}
		for(x=0;x<j-k;x++) {
			if ((strcmp(Argv[k+x],"<")==0))
				{
					op=1;
					myFile=Argv[k+x+1];
					if(k+x+2>=Argc)
						break;
					else
					{
						x++;
						continue;
					}
				}
			if ((strcmp(Argv[k+x],">")==0))
			{
				if(op==1)
					op=3;
				else
					op=2;
				myFile2=Argv[k+x+1];
				break;
			}
			args[x] = Argv[k+x];
		}
		
		// for(x=0; args[x]!=NULL;x++)  {
			// printf(args[x]);
		//}
		k = j+1;
//	} //Don't forget to erase!!!!!!!!!!!!!!!!!	
		//pipelining time!!!
		//if(op!=2)
			pipe(fd_new);
		pid = fork();
		if(pid==0)    //CHILD
		{
			if(op==1 ||op==3) {
				fd_old[0] = open(myFile, O_RDONLY);
			}
			if(op==2 ||op==3)
				fd_new[1] = open(myFile2, O_CREAT | O_WRONLY | O_TRUNC, 0666);
			if(i<pipes || op==2 ||op==3)
			{
				//reding from file
					close(fd_new[0]);
				dup2(fd_new[1], STDOUT_FILENO);
					close(fd_new[1]);
			}
			if((i!=0 || op==1)||op==3)
			{
				//writting into file
				if(op!=1 || op!=3)
					close(fd_old[1]);
				dup2(fd_old[0], STDIN_FILENO);
				if(op!=1 || op!=3)
					close(fd_old[0]);
			}
			if(execvp(args[0], args)==-1)
				perror("ERROR");
			exit(EXIT_SUCCESS);
		}
		else      //PARENT
		{
			if(i!=0)
			{
				close(fd_old[0]);
				close(fd_old[1]);
			}
			fd_old[0]=fd_new[0];
			fd_old[1]=fd_new[1];
			do{
				pid2 = waitpid(pid,&status,WNOHANG);
			} while(pid2!=pid);
		}
		for(x=0;x<10;x++) {
				args[x] = NULL;
		}
	}
	
	//SEAL the last one hahaha
		close(fd_old[0]);
		close(fd_old[1]);
}


int getInput2() {
	
	int counter = 0;
	char *buf;
	pipes = 0;
	while(Argc) {
		Argv[Argc] = NULL;
		Argc--;
	}
	input = getchar();
	while ((input != '\n')) {
		if(input == EOF) {
			printf("\n");
			exit(EXIT_SUCCESS);
		}
		if(input>=0 && input <=31) {
			input = ' ';
		};
		if((input == '<') || (input == '>') || (input == '|') || (input == '&')) {
			if(input == '<')
				in = 1;
			if(input == '>')
				out = 1;
			if(input == '|')
				pipes++;
			buffer[counter++] = ' ';
			buffer[counter++] = input;
			buffer[counter++] = ' ';
		}
		else {
			buffer[counter++] = input;
		}
		input = getchar();
	}
	
	buffer[counter]=0x00;
	//printf(buffer);
	//printf("\n");
	buf = strtok(buffer," \t");
	if(buf==NULL)
	{
		return -1;
	}
	while(buf != NULL) {
		Argv[Argc] = buf;
		buf = strtok(NULL," \t");
		Argc++;
	}
	return 0;
}


void shellPrompt(void) {
	if(prompt) {
		printf("sish :> ");
		fflush(stdout);
	}
}