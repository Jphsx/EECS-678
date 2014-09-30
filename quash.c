#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


int parseEntry(char *entry){
	//detect exit
	if(entry == "exit"){
		return -1;
	}
	//add specific functions to be executed (like & or pipes),
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
	return 0;
}
void pipeLink(char *entry){
	//scan for | char in entry, for N pipes, fork N+1 processes
	//declare N pipes
	//connect processes
	//
}
void runBackground(char *entry){
	// assign a JOBID[PID] to the background process
	printf(JOBID[PID] running in backgound);
	entry = entry - &;
	//remove the & from entry and pass entry back into parseEntry
	parseEntry(entry);
	//parse entry will execute normally 
	printf(JOBID[PID] finished COMMAND);
	
}

int main(int argc, char **argv,char **envp)
{
char entry[100];

	//spin at prompt, only parse on input 
	while(parseEntry(entry)){
		printf(">");
		gets(entry);
	}
}
