#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

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
int programRunning=1;
char cwd[1024];


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
			sleep(1);	
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
		if(kill(joblist[i].PID,0)==0){
   		         printf("[%d] %d running in background %s \n",joblist[i].JOBID,joblist[i].PID,joblist[i].COMMAND);
		}
		else{
			joblist[i].jobFlag=0;
		}
        }
        i=i+1;
    }

}

void dummyJob(){
	int k=5;
	while(k==6){};
}

void set(char* entry){
int ret;
int check;
char *args2=NULL;

	if(entry==NULL){
	}else if (strstr(entry, "HOME") != NULL){
		printf("No args\n");
		entry[strlen(entry) - 1] = '\0';
		args2 = strstr(entry, "=");
		args2 = args2 + 1;
		check = setenv("HOME",args2,1);
		printf("set HOME to:%s\ncheck:%i\n",args2,check);
	}else if (strstr(entry, "PATH") != NULL){
		entry[strlen(entry) - 1] = '\0';
		args2 = strstr(entry, "=");
		args2 = args2 + 1;
		printf("set PATH to:%s\n",args2);
		setenv("PATH",args2,1);
	}
}

void cd(char* entry){
int ret;
int check;
int test;

	if(entry==NULL){
		ret = chdir(getenv("HOME"));
	}else{
		entry[strlen(entry) - 1] = '\0';
		printf("change to directory:%s\n",entry);
		ret = chdir(entry);
		test =strcoll("test", entry);
		printf("ret:%i\ntest:%i\n",ret,test);
	}
}

void stdoutTofile(char *filename){
	FILE *outfile;
	outfile = freopen(filename, "w", stdout);
}

void stdoutToscreen(){
	freopen ("/dev/tty", "a", stdout);
}

void executeCommand(char *command, char* cmdbuffer, char* outFile) {
	pid_t execProcess;
	FILE *outfile;
	execProcess=fork();
	
	if(outFile!=NULL)
	outfile = freopen(outFile, "w", stdout);
	
	if(execProcess==0){	  
	 printf("executing command %s with parameters %s \n",command,cmdbuffer);
	
	if(cmdbuffer != NULL && outFile==NULL){	
		if(cmdbuffer[strlen(cmdbuffer)-1]=='\n'){
		cmdbuffer[strlen(cmdbuffer)-1]=NULL;
		}
		execlp(command,command,cmdbuffer,(char*)0);
	}
	else{
		if(command[strlen(command)-1]=='\n'){
		command[strlen(command)-1]=NULL;
		}
		execlp(command,command,(char*)0);
		
		//close(outfile);
	}
	}
}

int parseEntry(char*  entry){
	//detect exit
	char *command = NULL;
	char *args = NULL;
	char *entrydup = NULL;
	char* argsChopped = NULL;
	char* outFile = NULL;
			
	if(strstr(entry, " > ")!=NULL){
	outFile = strstr(entry, " > ");
	outFile = outFile+3;
	printf("%s\n",outFile);
		if(outFile[strlen(outFile)-1]=='\n'){
		outFile[strlen(outFile)-1]=NULL;
	}
	}
	
	if((strstr(entry, "quit") != NULL) || (strstr(entry, "exit") != NULL)){
		programRunning=0;
		printf("\n");
		exit(0);
	}

	// check for & to run in background
	else if(containsChar(entry,'&')!=-1){	
		runBackgroundTask(removeChar(entry,'&'));
	}
	//check for pipes in the command
   	 else if(containsChar(entry,'|')!=-1){
        //only will work for 1 pipe
 		entrydup = strdup(entry);
        	command = strtok(entry, "|");
                        args = strstr(entrydup, "|");
               	pipeLink(command,args);
	
    	}
	//check for jobs to print to screen
	else if((strstr(entry, "jobs") != NULL)){
		printJobList();
	}

	else if(strstr(entry, "set") != NULL){
		entrydup = strdup(entry);
		command = strtok(entry, " ");
		//args = strtok(entry, " ");
		//printf("%s\n",command);
		//printf("%s\n",args);
			if(strstr(entrydup, " ")!=NULL){
				args = strstr(entrydup, " ");
				argsChopped = args + 1;
			}
		//printf("%s\n",args);
		set(argsChopped);
	}
	
	else if(strstr(entry, "cd") != NULL){
		entrydup = strdup(entry);
		command = strtok(entry, " ");
		//args = strtok(entry, " ");
		//printf("%s\n",command);
		//printf("%s\n",args);
			if(strstr(entrydup, " ")!=NULL){
				args = strstr(entrydup, " ");
				argsChopped = args + 1;
			}
		//printf("%s\n",args);
		cd(argsChopped);
	}	


    else{
        entrydup = strdup(entry);
        command = strtok(entry, " ");
        if(strstr(entrydup," ")!=NULL){
        args = strstr(entrydup, " ");
		argsChopped=args+1;
		
		executeCommand(command,argsChopped,outFile);
	
	}
	else{
		args=NULL;
		executeCommand(command,args,outFile);
	}
	
        //executeCommand(command,args);   
    }

}

void pipeLink(char* input1,char* input2){
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
        parseEntry(input1);
        exit(0);
    }

    pid2 = fork();
    if(pid2 == 0) {
        dup2(fd1[0],0);
        close(fd1[0]);
        close(fd1[1]);

        parseEntry(input2);
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
		freopen ("/dev/tty", "a", stdout);
	    printf("PATH=%s\n",getenv("PATH"));
		printf("HOME=%s\n",getenv("HOME"));
		printf("JOBS RUNNING=%i\n",globalJobCounter);
		printf("%s>", getcwd(cwd, sizeof(cwd)));
		fgets(entry,100,stdin);
	   	parseEntry(removeChar(entry, '\n'));	
		
	}
}
