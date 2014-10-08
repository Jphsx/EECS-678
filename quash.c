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
struct commandContainer {
	char cmd[100];
	char cmdbuffer[100];
};

//global jobs array
struct job joblist[100];
int globalJobCounter=0;
int programRunning=1;

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
struct commandContainer* seperateCommand(char* command){
	struct commandContainer* comm;
	
	comm = (struct commandContainer*) malloc(sizeof(comm));	
	char tempCommand1[100];
	char tempCommand2[100];

	if(containsChar(command, ' ')!=-1){
		//splits the command and argument into pieces
		int spaceIndex=containsChar(command, ' ');
		int i=0;
		int j=0;
		while(i<spaceIndex){
			tempCommand1[i]=command[i];
			i=i+1;	
		}
		i=spaceIndex+1;
		while(i<strlen(command)-1){
			tempCommand2[j]=command[i];
			i=i+1;
			j=j+1;
		}
		strcpy(comm->cmd,tempCommand1);
		strcpy(comm->cmdbuffer,tempCommand2);
	}
	else{
		strcpy(comm->cmd,command);
		strcpy(comm->cmdbuffer," ");
	}
	return comm;
}
struct pipeTask seperatePipeTasks(char* command){
	struct pipeTask tasks;
	int indexOfPipebar;
	indexOfPipebar=containsChar(command,'|');
	char tempTask1[100],tempTask2[100];
	//pull out first task to temp array
	int i=0;
	while(i<indexOfPipebar-1){
		tempTask1[i]=command[i];
		i=i+1;
	}
	 i=indexOfPipebar+1;
	while(i<strlen(command)-1){
		tempTask2[i]=command[i];
		i=i+1;
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
	int counterCopy;
	counterCopy =  globalJobCounter;
	
	
	childpid = fork();
	struct job childJob;


	childJob.jobFlag=1;
	if(childpid==0){
		printf("[%d] %d running in background %s \n", counterCopy,getpid(),task);
		//pass task back to parse for execution
			sleep(2);	
		//	dummyJob();
		parseEntry(task);
		printf("[%d] %d finished in background %s \n", counterCopy,getpid(),task);
		//	joblist[counterCopy].jobFlag=0;
		exit(0);
	}
	else{
		childJob.JOBID = counterCopy;
		childJob.PID = childpid;
		strcpy(childJob.COMMAND, task);
		joblist[counterCopy] = childJob;
		globalJobCounter=globalJobCounter+1;
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
	char jobCommand[5];
	strcpy(jobCommand,"jobs\n");
	strcpy(killShellq,"quit\n");
	strcpy(killShelle,"exit\n");
	if(strcoll(killShelle, entry)==0 || strcoll(killShellq, entry)==0){
		programRunning=0;
		printf("\n");
		exit(0);
	}

	// check for & to run in background
	else if(containsChar(entry,'&')!=-1){	
		runBackgroundTask(removeChar(entry,'&'));
	}

	//check for jobs to print to screen
	else if(strcoll(jobCommand,entry)==0){
		printJobList();
	}

	//check for pipes in the command
	else if(containsChar(entry,'|')!=-1){
		//only will work for 1 pipe
		pipeLink(seperatePipeTasks(entry));
	}
	else{
		executeCommand(seperateCommand(entry));	
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
void executeCommand(struct commandContainer* comm) {
//	comm = (struct commandContainer*) malloc(sizeof(comm));	
	//do execl stuff
	printf("executing command %s with parameters %s \n",comm->cmd,comm->cmdbuffer);
//	free(comm->cmd);
//	free(comm->cmdbuffer);
	free(comm);
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
