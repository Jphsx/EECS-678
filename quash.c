#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

struct job {
	//job flag is 1 for process in progress, 0 for finished
	int jobFlag;
	int JOBID;
	int PID;
	char COMMAND[100];
};
//global jobs array
struct job joblist[100];


void runBackgroundTask(char* task){
  pid_t childpid;
       //put the background process in the array element of the parent ID
	childpid = fork();
	if(childpid==0){
		joblist[getppid()].jobFlag=1;
		printf("[%d] %d running in background %s",getppid(),getpid(),task);
		joblist[getppid()].JOBID=getppid();
		joblist[getppid()].PID= getpid();
		strcpy(joblist[getppid()].COMMAND, task);
		parseEntry(task);
	}
	else{
		printf("[%d] %d finished %s", joblist[getpid()].JOBID,joblist[getpid()].PID,joblist[getpid()].COMMAND);
		joblist[getpid()].jobFlag=0;
	}
	
}
 
int parseEntry(char*  entry){
	//detect exit
	 char quit[5];
	//put exit into exit char array to compare to entry
	strcpy(quit,"exit\n");
	
	if(strcoll(quit, entry)==0){
		printf("\n");
		exit(0);
	}
	
	// check for & to run in background
	char background[1];
	char modifiedEntry[100];
	strcpy(background, "&");
	
	if(strpbrk(background, entry) != NULL){
		//remove the &, fork and re parse
	//	strcpy(modifiedEntry,entry,strlen(entry)-1);
		runBackgroundTask(modifiedEntry);	

	}

/*	//add specific functions to be executed (like & or pipes),
	// otherwise execl with
	// command parameters loaded into buffer
	else if(entry contains | or >){
	//pipe outputs
		pipeLink(entry);
	}
	else if(entry contains &){
	//use a special fork method, and pass in entry
		runBackground(entry);
	}
	else{
		execl(entry,(char*)0);
	}
	return 1;
*/
}
void pipeLink(char *entry){
	//scan for | char in entry, for N pipes, fork N+1 processes
	//declare N pipes
	//connect processes
	//
}
void runBackground(char *entry){
/*	// assign a JOBID[PID] to the background process
	printf(JOBID[PID] running in backgound);
	entry = entry - &;
	//remove the & from entry and pass entry back into parseEntry
	parseEntry(entry);
	//parse entry will execute normally 
	//when finished print to screen
	printf(JOBID[PID] finished COMMAND);
*/	
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
