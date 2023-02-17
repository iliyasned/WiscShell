#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

char* paths[50];
int const PATHS_SIZE =50;


//source:https://codeforwin.org/2016/04/c-program-to-trim-leading-white-spaces-in-string.html
void trimLeading(char * str)
{
    int index, i, j;

    index = 0;

    /* Find last index of whitespace character */
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n')
    {
        index++;
    }


    if(index != 0)
    {
        /* Shit all trailing characters to its left */
        i = 0;
        while(str[i + index] != '\0')
        {
            str[i] = str[i + index];
            i++;
        }
        str[i] = '\0'; // Make sure that string is NULL terminated
    }
}

//source: https://stackoverflow.com/questions/3981510/getline-check-if-line-is-whitespace
int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}

//this function processes the commands
int process_command(char* com){
	
	char* comif = malloc(sizeof(char) * 512);
	char* comcopy = malloc(sizeof(char) *512);
	strcpy(comif, com);
	strcpy(comcopy, com);

	char* arg0;
    arg0 = strtok(comcopy, " \t");

	//checking for redirection
	int redir = 0;
    char* redirchar = strchr(com, '>');
    if(redirchar != NULL && strcmp(arg0, "if") != 0){
		redirchar++;
		trimLeading(redirchar);
		for(int i = 0; i < strlen(redirchar); ++i){    
			if(redirchar[i] == ' ' || redirchar[i] == '>'){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
				return -1;
			}   
		}
        redir = 1;
        com = strtok(com, ">");
    }
	

	arg0 = strtok(com, " \t");
	char* arg1;
	arg1 = strtok(NULL, " \t");
 
    //command has args
     if(arg1 != NULL){

		char* args[50];
		for(int i = 0; i < PATHS_SIZE; ++i){
			args[i] = (char*) malloc(sizeof(char) * 512);
			if(args[i] == NULL) {
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
				return -1;
			}
			strcpy(args[i], "");
			
		}
		strcpy(args[0],arg0);
		strcpy(args[1], arg1);
		
		char* arg2 = strtok(NULL, " \t");
        //printf("%s",arg2);
		
		//cd command
        if(strcmp(arg0,"cd")==0 && arg2 == NULL) {
			
			if(chdir(arg1) != 0){
				 char error_message[30] = "An error has occurred\n";
				 write(STDERR_FILENO, error_message, strlen(error_message)); 
                 return -1;
            }else return 0;
        }
		
		//path command
        else if(strcmp(arg0,"path")==0){
			if(arg2 != NULL) {
				strcpy(args[2], arg2);
				int i = 3;
				while(args[i-1] != NULL){
					args[i] = strtok(NULL, " \t");
					i++;
				}		
				for(int i = 1; i < PATHS_SIZE; i++){
					if(args[i] == NULL) break;
					if(strcmp(args[i], "./") == 0){
						args[i][strlen(args[i]) -1] = '\0';
						strcpy(paths[i-1], args[i]);
						continue;
					}
					strcpy(paths[i-1], args[i]);
				}
				//printf("%s\n", paths[0]);
			}
			else {
				if(strcmp(arg1, "./") == 0){
					arg1[strlen(arg1)-1] ='\0';
					strcpy(paths[0], arg1);
				}
				else{
					strcpy(paths[0], args[1]);
				}
				for(int i = 1; i < PATHS_SIZE; ++i) strcpy(paths[i], "");
			}
		}

		//if then command (can be nested)
		else if(strcmp(arg0, "if") ==0){
			int equals = 2;
			if(arg2 == NULL){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
				return -1;
			}
			
			
			char* comifcopy = malloc(sizeof(char) * 512);
			strcpy(comifcopy, comif);
			for(int i = 0; i < strlen(comif); ++i){
				if(comif[i] == '=' && comif[i+1] == '='){
					equals = 1;
					break;
				}
				else if(comif[i]=='!' && comif[i+1]=='='){
					equals = 0;
					break;
				}
				
			}
			if(equals == 2){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
				return -1;
			}

			char* com1 = strtok(comifcopy, " \t");
			com1 = strtok(NULL, "!=");
			int com1ret = 0;
			
			//run first command
			if(com1 != NULL){
				com1ret = process_command(com1);
			}else{
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
				return -1;
			}

			char* constant = malloc(sizeof(char)*512);
			int constnum = 0;
			strcpy(comifcopy, comif);

			if(equals == 1){
				constant = strtok(comifcopy, "!=");
				constant = strtok(NULL, " \t");
				constant = strtok(NULL, " \t");
				//printf("%s\n", constant);
				if(constant != NULL && isdigit(*constant) != 0){
					constnum = atoi(constant);
				}
				else{
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					return -1;
				}
			}
			else {
				constant = strtok(comifcopy, "!=");
				constant = strtok(NULL, " \t");
				constant = strtok(NULL, " \t");
				if(constant != NULL && isdigit(*constant) != 0){
					constnum = atoi(constant);
				} else{
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					return -1;
				}
			}
			
			//operation check
			if(equals == 1){
				if(constnum != com1ret) return 0;
			} else{
				if(constnum == com1ret) return 0;
			}
		
			//doing second command	
			char* com2 = malloc(sizeof(char)*512);
			strcpy(comifcopy, comif);
			int fifound = 0;
			if(equals == 1){
				strtok(comifcopy, "!=");
				strtok(NULL, " \t");
				strtok(NULL, " \t");
				char* test = strtok(NULL, " \t"); // now were at then
				if(strcmp(test, "then") != 0){
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					return -1;
				}

				char* temp = (char*)malloc(sizeof(char)*512);
				temp = strtok(NULL, " \t");
				//DO NOTHING
				if(strcmp(temp, "fi") == 0){
					return 0;
				}				


				while(temp != NULL){
					if(strcmp(temp,"fi_") == 0){
						char error_message[30] = "An error has occurred\n";
						write(STDERR_FILENO, error_message, strlen(error_message)); 
						return -1;
					}
					if(strcmp(temp, "fi") == 0){
						//checking if nested
						char* com2cp = malloc(sizeof(char) *512);
                        strcpy(com2cp, com2);
                        trimLeading(com2cp);
                        if(com2cp[0] == 'i' && com2cp[1] == 'f' ){
                            strcat(com2, " fi");
                        }

						fifound = 1; 
						break;
					}
					strcat(com2, " ");
					strcat(com2, temp);
					temp = strtok(NULL, " \t");
				}

				if(com2 == NULL){
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					return -1;
				}
			} else{
				strtok(comifcopy, "!=");
                strtok(NULL, " \t");
                strtok(NULL, " \t");
                char* test = strtok(NULL, " \t");
				if(strcmp(test, "then") != 0){
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					return -1;
				}

                char* temp = (char*)malloc(sizeof(char)*512);
				temp = strtok(NULL, " \n");
				//DO NOTHING
				if(strcmp(temp, "fi") == 0){
                    return 0;
                }

				
                while(temp != NULL){
					if(strcmp(temp,"fi_") == 0){
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return -1;
                    }

                    if(strcmp(temp, "fi") == 0){
						//checking if nested
						char* com2cp = malloc(sizeof(char) *512);
                        strcpy(com2cp, com2);
                        trimLeading(com2cp);
                        if(com2cp[0] == 'i' && com2cp[1] == 'f' ){
                            strcat(com2, " fi");
                        }

						fifound = 1;
						break;
					}
					strcat(com2, " ");
                    strcat(com2, temp);
					temp = strtok(NULL, " \t");
                }

                if(com2 == NULL){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                    return -1;
                }
				
			}

			if(fifound == 0){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
				return -1;
			}
			char* test = strtok(NULL, " \t");
			if(test != NULL){
				if( strcmp(test, "fi") != 0){
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					return -1;
				}
			} 
			
			process_command(com2);
			

		}

		//not built in command
		else{
			
			int redirindex = 0;
			int i = 3;
			if(arg2 != NULL){
				 strcpy(args[2] ,arg2);
				 //int i = 3;	
				while(args[i-1] != NULL){
					args[i] = strtok(NULL, " \t");
					i++;
				}
			}
		
			
			FILE* redirfp;
			//as long as file name is provided
			if(redir == 1 && *redirchar != '\0'){
				redirfp = fopen(redirchar, "w");
				if(redirfp == NULL){
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
					return -1;		
				}
			}
			//no file name provided 
			else if(redir==1) {
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));	
				return -1;
			}

			int fno = 0;
			if(redir == 1){
				fno = fileno(redirfp);
				
			}	
			
			char* binpath = malloc(sizeof(char) * 512);
			int workingpath = 0;
			for(int i = 0; i < PATHS_SIZE; ++i){
				
				if(*paths[i] == '\0') break;				
				strcpy(binpath, paths[i]);
				strcat(binpath, "/");
				strcat(binpath, arg0);
				if(access(binpath, X_OK) == 0) {
					workingpath = 1;
					break;
				}
			}

			if(!workingpath){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
				return -1;
			}
			
			

			int numarg = i;
	
			//argv setup for execv
			char* argv[numarg];
			argv[0] = malloc(sizeof(char) * 512);
			strcpy(argv[0], arg0);
							
			for(int i = 1; i < numarg; ++i){
				if(i == numarg-1){
					argv[i] = NULL;
					break;
				}
				argv[i] = malloc(sizeof(char*) * 512);
				if(argv[i] == NULL){
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					return -1;
				}
				strcpy(argv[i], args[i]);
			}


            pid_t pid = fork();
            if(pid==0){
			
					
				if(redir == 1) dup2(fno, STDOUT_FILENO);
                execv(binpath ,argv);

                char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
                exit(1);
            }
			//waiting for child
            int status;
            waitpid(pid, &status, 0);
			int returnstatus = WEXITSTATUS(status);
			//close redir file if redir mode
			if(redir == 1 && redirfp != NULL) fclose(redirfp);
			return returnstatus;
		}

    }

	//no arg command
    else if(arg1 == NULL){
        if(strcmp(arg0, "exit") == 0){
            free(com);
            exit(0);
        }

		//empty the paths
		else if(strcmp(arg0, "path")==0) {
			for(int i = 0; i < PATHS_SIZE; ++i) strcpy(paths[i], "");
			return 0;
		}

		//not built in command
        else{
			
		//TODO figure out what is the max strlen of paths
			FILE* redirfp;
            //as long as file name is provided
            if(redir == 1 && *redirchar != '\0'){
                redirfp = fopen(redirchar, "w");
                if(redirfp == NULL){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return -1;
                }
            }
            //no file name provided 
            else if(redir==1) {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                return -1;
            }

            int fno = 0;
            if(redir == 1){
                fno = fileno(redirfp);
            }
	
			char* binpath = malloc(sizeof(char)*512);
            int workingpath = 0;
            for(int i = 0; i < PATHS_SIZE; ++i){

				if(*paths[i] == '\0') break;

                strcpy(binpath, paths[i]);
				strcat(binpath, "/");
                strcat(binpath, arg0);

                if(access(binpath, X_OK) == 0) {
                    workingpath = 1;
                    break;
                }

            }

            if(!workingpath){
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
				return -1;
            }

            pid_t pid = fork();
            if(pid==0){
                char *const argv[2] = {
					arg0, // string literial is of type "const char*"
                    NULL // it must have a NULL in the end of argv
                };
				
				if(redir == 1) dup2(fno, STDOUT_FILENO);
                execv(binpath,argv);
				
				//failed execv
                char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message)); 
                exit(1);
            }
            int status;
            waitpid(pid, &status, 0);
			int returnstatus = WEXITSTATUS(status);
			if(redir == 1 && redirfp != NULL) fclose(redirfp);
			return returnstatus;
        }
    }

	//error
	else {
		char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
		
}

int main(int argc, char** argv){

	char* buf;
	size_t bufsize = 0;
	buf = (char*) malloc(bufsize * sizeof(char));
	int batchmode = 0;

	for(int i = 0; i < PATHS_SIZE; ++i){
		*(paths+i) = (char*) malloc(sizeof(char) * 512);
		
		//could not allocate memory
		if(*(paths+i) == NULL){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message)); 
			return -1;
		}
	}
	strcpy(paths[0],"/bin");	
	
	//more than one file
	if(argc > 2) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message)); 
		exit(1);
	}
	
	//batch mode	
	if(argc == 2) batchmode = 1;
	
	if(buf == NULL){
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message)); 
		exit(1);
	}
	
	if(!batchmode){
		while(1){

			printf("wish>");
		
			size_t input = getline(&buf, &bufsize, stdin);
			//remove newline char
			if(strlen(buf) != 0 && buf[strlen(buf)-1] == '\n') buf[strlen(buf) - 1] = 0;	
	
			if(buf == NULL || *buf == '\0' || is_empty(buf) == 1) continue;

			process_command(buf);
		}
	}
	//DONE implement batch mode
	else{
		FILE* file = fopen(argv[1] , "r");
		ssize_t read;

		//bad file
		if(file == NULL){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message)); 
			exit(1);
		}

		while((read = getline(&buf, &bufsize, file)) != -1){
			if(strlen(buf) != 0 && buf[strlen(buf)-1] == '\n') buf[strlen(buf) - 1] = 0;
			if(buf == NULL || *buf == '\0' || is_empty(buf) == 1) continue;
			process_command(buf);
		}
		fclose(file);
	}
	return 0;
}
