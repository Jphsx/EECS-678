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
	
	
}

int main(int argc, char *argv[])
{
char entry[100];

	//spin at prompt, only parse on input 
	while(parseEntry(entry)){
		printf(">");
		gets(entry);
	}
}
