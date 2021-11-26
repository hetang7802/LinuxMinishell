#include <bits/stdc++.h>
#include<unistd.h>
#include<sys/wait.h>
#include<pwd.h>
#include<fcntl.h>

#define clearPrompt() printf("\033[H\033[J")
#define BOLDCYAN    "\033[1m\033[36m"      // Bold Cyan color for prompt
#define RESET   "\033[0m"  				// reset to default color
#define MAGENTA "\033[35m"      /* Magenta */

//saving the standard output and standard input for future use
int saved_stdout=dup(STDOUT_FILENO);
int saved_stdin=dup(STDIN_FILENO);

using namespace std;

// storing the username and home directory for future reference
char* username=getenv("USER");
const char* homeDir=getpwuid(getuid())->pw_dir;

//making a vector for history
vector<string> history;

// making a struct to handle the parsed commands
struct command{
	int numberOfParameters;
	char* parameters[1000];
};

// making a struct to handle the pipes if given in input
struct forPipe{
	char* input1;
	char* input2;
};

// print a welcome prompt at the start
void welcome(){
	cout << MAGENTA;
	clearPrompt();
    printf("\n\t*************initialising minishell please wait************\n");
    printf("\n\t\t           made by: hetang\n");
    printf("\nUSER is: @%s", username);
    printf("\n\n\n");
    sleep(1.5);
    cout << RESET;
}	

// showing the current directory
void showDirectory(){
	//cout << "\033[1;31mbold red text\033[0m\n";
	dup2(saved_stdout,STDOUT_FILENO);
	char temp[1024];
	getcwd(temp, sizeof(temp));
	cout << BOLDCYAN << "dir: " << temp << "> " << RESET ;
}

void inputCommand(char* buffer){
	fgets(buffer, 2000, stdin);
	buffer[strlen(buffer)-1]='\0';
	int i=1;
	while(buffer[strlen(buffer)-i]==' ' || buffer[strlen(buffer)-i]=='\t' ){
		buffer[strlen(buffer)-i]='\0';
		i++;
	}
}

int parseCommand(char* inputLine, struct command* cmd){
	if(inputLine=="\n"){
		cmd->numberOfParameters=0;
		return 0;
	}
	const char* delim="\t\n<>= ";
	char **ptr=&inputLine;
	int noOfPara=0;
	char* field;
	while((field = strsep(&inputLine,delim)) != NULL){
		int i=1;
		while(field[strlen(field)-i]==' ' || field[strlen(field)-i]=='\t' ){
			field[strlen(field)-i]='\0';
			i++;
		}
		if(field[0]!='\0'){
			cmd->parameters[noOfPara++]=field;
			//printf("field: %s\n",field);
		}
	};
	cmd->parameters[noOfPara]=nullptr;
	cmd->numberOfParameters=noOfPara;
	
	//return 0 for successful parsing
	return 0;
}

int parsePipe(char* inputLine,struct forPipe* pipe){
	const char* delim="|\n";
	char **ptr=&inputLine;
	int noOfPara=0;
	char* field;
	int check=0;
	while((field = strsep(&inputLine,delim)) != NULL){
		int i=1;
		while(field[strlen(field)-i]==' ' || field[strlen(field)-i]=='\t' ){
			field[strlen(field)-i]='\0';
			i++;
		}
		if(field[0]!='\0'){
			if(check==0){pipe->input1=field;}
			else{pipe->input2=field;};
			check=1;
			//printf("field for pipe: %s\n",field);
		}
	};
	//return 0 for successful parsing
	return 0;
}

//making function to add to history
void add_history(string str){
	history.push_back(str);
	return;
}

//to print the history
void prntHis(){
	for(int i=0;i<history.size();i++){
		cout << endl  << " " << i+1 << " " << history[i] ;
	}
	cout << endl;
	return;
}

int main(int argc, char* argv[]){
	
	command curr_command;// curr_command will store the parameters in inputline
	
	char inputLine[2000];

	//print the starting prompt 
	welcome();
	
	while(true){
		
		//show the current directory
		showDirectory();

		//read the command line
		inputCommand(inputLine);
		
		//check if the inputLine is history or is blank		
		if(inputLine[0]!='\0'){
			string s(inputLine);
			add_history(s);
			if(s=="history"){
				cout << "the history is:" ;
				prntHis();
				continue;
			}
		}else{
			continue;
		}
		
		if((strcmp(inputLine, "exit") == 0) ||
			(strcmp(inputLine, "quit") == 0))
       	{
       		// clearing the history at the end of program
       		history.clear();
       		exit(0);
       		continue;
		}
		
		//check if there is call for env variable or echo
		if(string(inputLine).find("echo ")!=string::npos){
			char tempInputLine[2000];
			strcpy(tempInputLine,inputLine);
			command tempCommand;
			parseCommand(tempInputLine,&tempCommand);
			if(tempCommand.parameters[1][0]=='$' && tempCommand.parameters[1][1]!=' '){
				const char* envVar=tempCommand.parameters[1]+1;
				cout << getenv(envVar) << endl;
				continue;
			}
		}
		if(string(inputLine).substr(0,6)=="setenv"){
			parseCommand(inputLine,&curr_command);
			//we will handle setenv only for five env variables
			const char* name= curr_command.parameters[1];
			const char* value= curr_command.parameters[2];
			if(setenv(name,value,1)!=0)cout << "error while handling env variable" << endl;
			continue;
		}
		
		//checking for pipe
		int pipeCheck=0;
		for(int i=0;i<strlen(inputLine);i++){
			if(inputLine[i]=='|'){
				pipeCheck=1;
				break;
			}
		}
		
		// code if there is piping involved
		if(pipeCheck==1){
			forPipe pipeCommands;
			parsePipe(inputLine,&pipeCommands);// parsing the piped input
			command command1,command2; //declare two commands to store the pipe commds
			parseCommand(pipeCommands.input1,&command1);
			parseCommand(pipeCommands.input2,&command2);
			if(fork()==0){
				int newstdout1=open("tempFileForPipe",O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
				//shift the output to temporary file
				dup2(newstdout1,STDOUT_FILENO);
				cout << clearPrompt();
				if(execvp(command1.parameters[0],command1.parameters)<0){
					cout << "error while execution, please check command" << endl;
				};
				//perror("execvp");
				cout << '\0';
				dup2(saved_stdout,STDOUT_FILENO);
				exit(0);
			}else{
				wait(NULL);
				int newstdin1 = open("tempFileForPipe",O_RDONLY);
				dup2(newstdin1,STDIN_FILENO); //shift the std input
				char buf[1000];
				string s="tempFileForPipe";
				char* cstr = (char*)(s.c_str());
				command2.parameters[command2.numberOfParameters++]=cstr;
				dup2(saved_stdin,STDIN_FILENO);
				if(fork()==0){
					command2.parameters[command2.numberOfParameters]=nullptr;
					dup2(saved_stdout,STDOUT_FILENO);//change the std output again to terminal
				//cout << "the parameters are "<< command2.parameters[0]<< endl;
					if(execvp(command2.parameters[0],command2.parameters)<0){
						cout << "error while execution, please check command" << endl;
					};
					//perror("execvp");
					exit(0);
				}else{
					int status=0;
					pid_t pid;
					wait(&status);
				if(status!=0)cout <<"failed to run command correctly\tplease check the command"<< endl;
				}
				//wait for the child process to complete
				int status=0;
				pid_t pid;
				wait(&status);
			}
			//removing the temp file created for pipe
			remove("tempFileForPipe");
			continue;
		}
		
		// checking if there is request for redirection
		bool in=false,out=false,append=false;
		for(int i=0;i<strlen(inputLine);i++){
			switch(inputLine[i]){
				case '<':in=true;break;
				case '>':{if(inputLine[i+1]=='>'){
								append=true;
								i++;
							}else{
								out=true;
							}	
							break;
						}
				default: continue;
			}
		}		
		
		// parsing the commands
		parseCommand(inputLine,&curr_command);
		// declaring a string to store the file name of
		// of the file to be redirected in
		string fileName;
		if(in){
			if(append || out){
				int newstdout;
				string outFile=curr_command.parameters[curr_command.numberOfParameters-1];
				if(out){
					newstdout=open(&outFile[0],O_WRONLY|O_TRUNC|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
				}else{
				    newstdout = open(&outFile[0],O_WRONLY|O_APPEND|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
				}
				dup2(newstdout,STDOUT_FILENO);
				fileName=curr_command.parameters[curr_command.numberOfParameters-2];
				// remove the filename from the list of commands
				curr_command.parameters[curr_command.numberOfParameters-1]=nullptr;
				char* cstr=(char*)fileName.c_str();
				curr_command.parameters[curr_command.numberOfParameters-2]=cstr;
			}else{
				fileName=curr_command.parameters[curr_command.numberOfParameters-1];
				// remove the filename from the list of commands
				curr_command.parameters[curr_command.numberOfParameters-1]=nullptr;
				char* cstr=(char*)fileName.c_str();
				curr_command.parameters[curr_command.numberOfParameters-1]=cstr;
			}
			
			dup2(saved_stdin,STDIN_FILENO);
			if(fork()==0){
				//cout << curr_command.parameters[0] << curr_command.parameters[1] << endl;
				if(execvp(curr_command.parameters[0],curr_command.parameters)<0){
					cout << "error while execution, please check command" << endl;
				};
				//perror("execvp");
				exit(0);
				//cout << "now back to terminal" << endl;
			}else{
				int status=0;
				pid_t pid;
				wait(&status);
				if(status!=0)cout << "failed to run command correctly\tplease check the command" << endl;
			}
		}else if(append){
			//we assume the redirection can will be give at the end of command
			fileName=curr_command.parameters[curr_command.numberOfParameters-1];
			// remove the filename from the list of commands
			curr_command.parameters[curr_command.numberOfParameters-1]=nullptr;
			if(fork()==0){
				int newstdout = open(&fileName[0],O_WRONLY|O_APPEND|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
				dup2(newstdout,STDOUT_FILENO);
				if(execvp(curr_command.parameters[0],curr_command.parameters)<0){
					cout << "error while execution, please check command" << endl;
				};
				//perror("execvp");
				exit(0);
				dup2(saved_stdout,STDOUT_FILENO);
				//cout << "now back to terminal" << endl;
			}else{
				int status=0;
				pid_t pid;
				wait(&status);
				if(status!=0)cout << "failed to run command correctly\tplease check the command" << endl;
				
			}
		}else if(out){
			//we assume the redirection can will be give at the end of command
			fileName=curr_command.parameters[curr_command.numberOfParameters-1];
			// remove the filename from the list of commands
			curr_command.parameters[curr_command.numberOfParameters-1]=nullptr;
			if(fork()==0){
				int newstdout = open(&fileName[0],O_WRONLY|O_TRUNC|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
				dup2(newstdout,STDOUT_FILENO);
				cout << clearPrompt();
				if(execvp(curr_command.parameters[0],curr_command.parameters)<0){
					cout << "error while execution, please check command" << endl;
				};
				//perror("execvp");
				exit(0);
				dup2(saved_stdout,STDOUT_FILENO);
				//cout << "now back to terminal" << endl;
			}else{
				int status=0;
				pid_t pid;
				wait(&status);
				if(status!=0)cout << "failed to run command correctly\tplease check the command" << endl;
			}
		}else{
			if(fork()==0){
				if(execvp(curr_command.parameters[0],curr_command.parameters)<0){
					cout << "error while execution, please check command" << endl;
				};
				//perror("execvp");
				exit(0);
			}else{
				int status=0;
				pid_t pid;
				wait(&status);
				if(status!=0)cout << "failed to run command correctly\tplease check the command" << endl;
				
			}
		}
		
		
		// checking for change directory request 
		if(inputLine[0]=='c'&&inputLine[1]=='d'){
			string s = inputLine;
			if(inputLine[3]=='\0'){
				chdir(homeDir);
			}else{
				if(inputLine[3]!=' ' || inputLine[3]!='\t'){
					cout << "please enter valid command" << endl;
				}
				chdir(curr_command.parameters[1]);
			}
			continue;
		}else{
			int check=0;
			bool in=false,out=false,append=false;
			for(int i=0;i<strlen(inputLine);i++){
				switch(inputLine[i]){
					case '<':in=true;check=1;break;
					case '>':{if(inputLine[i+1]=='>')append=true;
								else out=true;
								check=1;
								break;
							}
					default: continue;
				}
				if(check=1)break;
			}
		}
	}	
}
