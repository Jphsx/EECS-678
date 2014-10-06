#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>

struct job {
	//job flag is 1 for process in progress, 0 for finished
	int jobFlag;
	int JOBID;
	int PID;
	char COMMAND[100];
};
struct pipeTask {
	char task1[100];
	char task2[100];
};
//global jobs array
struct job joblist[100];
int globalJobCounter=0;
int programRunning=1;


struct pipeTask seperatePipeTasks(char* command){
	struct pipeTask tasks;
	int indexOfPipebar;
	indexOfPipebar=containsChar(command,'|');
	char tempTask1[100],tempTask2[100];
	//pull out first task to temp array
	int i=0;
	while(i<indexOfPipebar-1){
		tempTask1[i]=command[i];
	}
	 i=indexOfPipebar+1;
	while(i<strlen(command)-1){
		tempTask2[i]=command[i];
	}
	strcpy(tasks.task1,tempTask1);
	strcpy(tasks.task2,tempTask2);
	return tasks;	

}

char* removeChar(char*string, char character){
	int i=0;
	while(i<strlen(string)-1){
		if(string[i]==character){
			string[i]='\n';
			string[i+1]=NULL;
		}
		i=i+1;
	}
	return string;
}

//return index if a character is in a string, and -1 if it is not
int containsChar(char* string, char character){
	int i=0;
	while(i<strlen(string)-1){
		if(string[i]==character){
			return i;
		}
		i=i+1;
	}
	return -1;
}
//takes a task and splits off a child process to execute it
//stores the job information in the struct job array
//for later access
//NOTE: this doesntwork properly, cant reset job flag yet so jobs command will
// print every job ever run in the background (might have to rewrite with threads 
// because child process can not access global variables and waiting 
// on the parent process causes the program to hang)
void runBackgroundTask(char* task){
	pid_t childpid;
	if(globalJobCounter == 99){
		globalJobCounter = 0;
	}
	// store the counter incase it is changed during execution, increment
	int counterCopy = globalJobCounter;
	int status;
	childpid = fork();
	struct job childJob;
	childJob.jobFlag=1;

	if(childpid==0){
		printf("[%d] %d running in background %s \n", counterCopy,getpid(),task);
		//pass task back to parse for execution
		//		dummyJob();
		parseEntry(task);
		printf("[%d] %d finished in background %s \n", counterCopy,getpid(),task);
		//	joblist[counterCopy].jobFlag=0;
	}
	else{
		childJob.JOBID = counterCopy;
		childJob.PID = childpid;
		strcpy(childJob.COMMAND, task);
		joblist[counterCopy] = childJob;
		globalJobCounter++;
	}	
}
//iterates through the job stack and checks 
//for any job in progress flag then prints
//out all the information for each background job
void printJobList(){
	int i=0;
	while( i<99) {
		if(joblist[i].jobFlag == 1){
			printf("[%d] %d running in background %s \n",joblist[i].JOBID,joblist[i].PID,joblist[i].COMMAND);
		}
		i=i+1;
	}

}
void dummyJob(){
	int k=5;
	while(k==6){};
}

int parseEntry(char*  entry){
	//detect exit
	char killShelle[5];
	char killShellq[5];
	strcpy(killShellq,"quit\n");
	strcpy(killShelle,"exit\n");
	if(strcoll(killShelle, entry)==0 || strcoll(killShellq, entry)==0){
		programRunning=0;
		printf("\n");
		exit(0);
	}

	// check for & to run in background
	if(containsChar(entry,'&')!=-1){	
		runBackgroundTask(removeChar(entry,'&'));
	}

	//check for jobs to print to screen
	char jobCommand[5];
	strcpy(jobCommand,"jobs\n");
	if(strcoll(jobCommand,entry)==0){
		printJobList();
	}

	//check for pipes in the command
	if(containsChar(entry,'|')){
		//only will work for 1 pipe
		pipeLink(seperatePipeTasks(entry));
	}

}
void pipeLink(struct pipeTask tasks){
	pid_t pid1,pid2;
	int status, fd1[2];
	//set up pipes
	pipe(fd1);

	pid1 = fork();
	if (pid1 == 0) {
		dup2(fd1[1],1);
		close(fd1[0]);
		close(fd1[1]);

		//send task to parse which will then call execute
		//this is because we might want to pipe with 
		//a special command like 'jobs'
		parseEntry(tasks.task1);
		exit(0);
	}

	pid2 = fork();
	if(pid2 == 0) {
		dup2(fd1[0],0);
		close(fd1[0]);
		close(fd1[1]);

		parseEntry(tasks.task2);
		exit(0);
	}
	close(fd1[0]);
	close(fd1[1]);	
}

int main(int argc, char **argv,char **envp)
{
char entry[100];
  
	//spin at prompt, only parse on input 
	while(programRunning){
		printf(">");
		fgets(entry,100,stdin);
	   	 parseEntry(entry);
		
	}
}
