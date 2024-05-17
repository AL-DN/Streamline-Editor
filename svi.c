#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STRING 256
#define MAX_COMMAND 100
#define MAX_IO_LEN 256
typedef char String[MAX_STRING];

typedef enum {
	range,
	text,
	none
} LineRangeType;

typedef union {
	//if no line range
	int none;
	//if text
	String commandText;
	//if range (2-1) = # of lines needed to read
	int range[2];
} LineRangeSpec; 

typedef struct {

LineRangeType lineRange;
LineRangeSpec spec;
char commandChar;
String data;

} Command;


//-------FUNCTIONS -----//

Command getCommand(String commandBuffer) {
	
	Command tempCommand;
	
	// LineRangeType = Text
	if(commandBuffer[0]=='/') {
		//LineRangeType SET
		tempCommand.lineRange=text;
		char *start = commandBuffer+1;
		char *end = strchr(start,'/');		
		if(end != NULL && end-start <= 80) {
			size_t length = end - start;
			//LineRangeSpec SET
			strncpy(tempCommand.spec.commandText,start,length);
			tempCommand.spec.commandText[length]='\0';
			char *operationptr=end+1;
			strncpy(commandBuffer,operationptr,strlen(operationptr)+1);
			//commandChar SET
			tempCommand.commandChar=commandBuffer[0];			
			//Data SET
			strncpy(tempCommand.data,start,strlen(commandBuffer));
			
				 
		} else { printf("Format: /<text>/ & text<=80 chars"); }
	}
	
	//LineRageType = range(specifies line numbers)
	else if(isdigit(commandBuffer[0])) {
		//LineRangeType SET
		tempCommand.lineRange=range;
		//LineRangeSpec SET
		//comma sperator between line specification
		char *lineF =  &commandBuffer[0];
		char *lineL =  &commandBuffer[2];
		tempCommand.spec.range[0] = atoi(lineF);
		tempCommand.spec.range[1] = atoi(lineL);
		//commandChar SET
		char *operationptr = strstr(commandBuffer,"/")+1;
		strncpy(commandBuffer,operationptr,strlen(commandBuffer));
		
		tempCommand.commandChar = commandBuffer[0];
		strncpy(tempCommand.data,commandBuffer+1,strlen(commandBuffer));
	}

	else {
		//LineRageType = none
		//LineRangeType SET
		tempCommand.lineRange = none;
		//LineRangeSpec SET
		tempCommand.spec.none = 1;
		//commandChar SET
		tempCommand.commandChar = commandBuffer[0];
		//Data SET
		strncpy(tempCommand.data,commandBuffer+1,strlen(commandBuffer));
	}
	return tempCommand;
} // end of getCommand()

int commandApplies(Command tempCommand, String stdBuffer,int lineCount) {
	switch(tempCommand.lineRange) {
		case text:
			//looking for /<text>/ within the given input str
			if(strstr(stdBuffer,tempCommand.spec.commandText)!=NULL) {
				return 1;
			}
			break;
	
		case range:
			// if within line numbers specifies by command
			if(lineCount>= tempCommand.spec.range[0] && lineCount <= tempCommand.spec.range[1]) { 
			return 1;
			}
			break;
		// do command in all cases
		case none:
			return 1;
			break;
		default:
			break;
	}
	// if none were chosen do not do command 
	return 0;
} // end of commandApplies()

void replaceSubstring(char *string,char* substring,char* replacement) {
	char *subStrSrc=strstr(string,substring);
	if(subStrSrc == NULL ) {
		return;	
	}
	
	memmove(subStrSrc+strlen(replacement) , // where to copy 
		subStrSrc + strlen(substring) , // from where to cpy
		strlen(subStrSrc)-strlen(substring)+1); // size
	// use memcpy to exlude null terminator
	memcpy(subStrSrc,replacement,strlen(replacement));
}

void executeCommand(Command c, String buffer) {
	//REMINDERS:
	// data: the text you will be appending, inserting etc
	// where that goes is based on commandChar
	switch(c.commandChar) {
		case 'A': //1
			//add the data to end of buffer; const = 256 char
			strncat(buffer,c.data,MAX_STRING-strlen(buffer));
			break;
		case 'I': //2
			//add data to start of buffer; const = 256 char
			String temp;
			//data first (gave priority of space in array to command data)
			strncat(temp,c.data,strlen(c.data));
			//adds buffer;
			strncat(temp,buffer,MAX_STRING - strlen(buffer));
			strncpy(buffer,temp,MAX_STRING);
			break;
		case 'O': //3
			//create new line with data
			printf("%s\n",c.data);
			break;
		//d is avoided
		case 'S': //5
			//REMINDERS:
			//commandText will hold str we are searching for
			//data needs to be parsed and split into /<oldtext>/<newtext>/
			//replace oldtext with new text			
			String data2;
            		memcpy(data2,c.data,sizeof(String));
            		//split into /<oldtext>/<newtext>/
            		char *oldtext=strtok(data2,"/");
            		char *newtext=strtok(NULL,"/");
	
			replaceSubstring(buffer,oldtext,newtext);		
			break;
	}
}


	
	
int main(int argc, char *argv[]) {


//------- READ FILE -------------//
	
	//correct input format
 		if(argc != 2){
		printf("Correct Format is: ./svi <commandFile>\n");
		exit(EXIT_FAILURE);
		}
	
	
	//reads file with commands
	FILE* commandFile = fopen(argv[1],"r");
	
	if(commandFile == NULL) {
		printf("No file could be read!");
		exit(EXIT_FAILURE);
	}
		
	int commandCount=0;
	int *ccp = &commandCount;
	Command CommandArray[MAX_COMMAND];
	
	String commandBuffer;
	while(fgets(commandBuffer,MAX_IO_LEN,commandFile)!=NULL) {
		if(commandCount == MAX_COMMAND){
			printf("MAX COMMANDS REACHED");
			break;
		} else {
	
		CommandArray[commandCount]=getCommand(commandBuffer);
		commandCount++;
		
		}
	}

        fclose(commandFile);
	//printf("%s",CommandArray[4].spec.commandText);
	//printf("%c",CommandArray[4].commandChar);
	//printf("%s",CommandArray[4].data);
// ------ READ FROM STDIN ------//

	String buffer;
	int lineCount=1;
	// for each line read
	while(fgets(buffer,MAX_IO_LEN,stdin)!=NULL) {
		//remove newline
		buffer[strlen(buffer)-1]= '\0';
		int delete = 0;
		//iterate though array of commands
		for(int i=0;i<commandCount;i++) { 
			// if this stdin is within parameters of command, do the command
                       	
			if(commandApplies(CommandArray[i],buffer,lineCount)){ 
				if(CommandArray[i].commandChar =='d') {
					delete=1;
					break;
				}
				executeCommand(CommandArray[i],buffer);
			}
		
		}
		
		if (buffer[strlen(buffer)-1] == '\n') { 
			buffer[strlen(buffer)-1] = '\0';		
		}
		// if line is not deleted, print line
		if (delete == 0) 
			printf("%s\n", buffer);
	

		lineCount++;
	}

return 0;

}
