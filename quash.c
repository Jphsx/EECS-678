#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

struct job {
	//job flag is 1 for process in progress, 0 for finished
	int jobFlag;
	int JOBID;
	int PID;
	char COMMAND[100];
};
//global jobs array
struct job joblist[100];
int globalJobCounter=0;

char* removeChar(char*string, char character){
	int i=0;
	while(i<strlen(string)-1){
		if(string[i]==character){
			string[i]='\0';
		}
	i=i+1;
	}
	return string;
}

//return 0 if a character is in a string, and 1 if it is not
int containsChar(char* string, char character){
	int i=0;
	while(i<strlen(string)-1){
		if(string[i]==character){
			return 0;
		}
	i=i+1;
	}
	return 1;
}
//takes a task and splits off a child process to execute it
//stores the job information in the struct job array
//for later access
void runBackgroundTask(char* task){
	  pid_t childpid;
	
	if(globalJobCounter == 99){
		globalJobCounter = 0;
	}
// store the counter incase it is changed during execution, increment
	int counterCopy = globalJobCounter;
	globalJobCounter++;

	int childReturnValue=0;

	childpid = fork();
	if(childpid==0){
		
		printf("[%d] %d running in background %s \n", counterCopy,getppid(),task);
		//pass task back to parse for execution
		dummyJob();
		
		printf("[%d] %d finished in background %s \n", counterCopy,getppid(),task);
		joblist[counterCopy].jobFlag=0;
	}
	else{
		//parent process populates job struct and pushes onto job stack
		struct job childJob;
		childJob.jobFlag = 1;
		childJob.JOBID = counterCopy;	
		childJob.PID = getpid();
		strcpy(childJob.COMMAND, task);
		joblist[counterCopy] = childJob;
	}	
}
//iterates through the job stack and checks 
//for any job in progress flag then prints
//out all the information for each background job
void printJobList(){
	int i=0;
	while( i<99) {
		if(joblist[i].jobFlag == 1){
			printf("[%d] %d running in background %s",joblist[i].JOBID,joblist[i].PID,joblist[i].COMMAND);
		}
		i=i+1;
	}
	
}
void dummyJob(){
	int k=5;
	while(k==5){};
}
 
int parseEntry(char*  entry){
	//detect exit
	 char quit[5];
	strcpy(quit,"exit\n");
	if(strcoll(quit, entry)==0){
		printf("\n");
		exit(0);
	}
	
	// check for & to run in background
	if(containsChar(entry,'&')==0){	
	runBackgroundTask(removeChar(entry));
	}

	//check for jobs to print to screen
	char jobCommand[5];
	strcpy(jobCommand,"jobs\n");
	if(strcoll(jobCommand,entry)==0){
		printJobList();
	}
}
void pipeLink(char *entry){
	//scan for | char in entry, for N pipes, fork N+1 processes
	//declare N pipes
	//connect processes
	//
}

int main(int argc, char **argv,char **envp)
{
char entry[100];
  
	//spin at prompt, only parse on input 
	while(1){
		printf(">");
		fgets(entry,100,stdin);
	   	 parseEntry(entry);
		
	}
}
