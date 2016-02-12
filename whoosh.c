#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

char error_message[30] = "An error has occurred\n";

char ** tokenize (char * line) {
    char ** theCmd = malloc(128 * sizeof(char*));
    char * token = strtok (line, " \n");
    int i = 0;
    while (token != NULL) {
        theCmd[i] = malloc (strlen(token) + 1);
        strcpy (theCmd[i], token);
	i++;
        token = strtok(NULL, " \n");
    }
    theCmd[i] = NULL;
    return (theCmd);
}

struct stat fileStat;

void redirect (char ** path, char ** theCmd) {
    char output[128];
    char error[128];
    char thePath[128];
    int i = 0;
    int success = 0;
    int pid;
    int index = 0;
    while (theCmd[i] != NULL) {
        if ((strcmp(theCmd[i], ">")) == 0) {
            index = i + 1;
        }
        i++;
    }
    i = 0;
    strcpy(output, theCmd[index]);
    strcat(output, ".out");
    strcpy(error, theCmd[index]);
    strcat(error, ".err");
    while(path[i] != NULL) {
        strcpy(thePath, path[i]);
        strcat(thePath, "/");
        strcat(thePath, theCmd[0]);
        if ((stat(thePath, fileStat)) == -1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else {
            break;
        } 
        i++;
    }
    pid = fork();
    if (pid == 0){ 
        close(STDOUT_FILENO);
        if ((open(output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH)) < 0) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }
        close(STDERR_FILENO);
        if ((open(error, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH)) < 0) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }            
        if((execv(thePath, theCmd)) == -1) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }
    }
    else if (pid < 0) {
        write (STDERR_FILENO, error_message, strlen(error_message));
    }
    else {
        if((wait(&pid)) < 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

int checkRedirect (char ** theCmd) {
    int i = 0;
    int count = 0;
    int index = 0;
    int redirect = 0;
    while (theCmd[i] != NULL) {
        if ((strcmp(theCmd[i], ">")) == 0) {
            count++;
            index = i + 1;
        }
        i++;
    }
    if (count == 1) {
        if(theCmd[index] == NULL) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }
        else {
            redirect = 1;
        }
    }
    else if (count > 1) {
        write (STDERR_FILENO, error_message, strlen(error_message));
    }
    else {
        if(theCmd[index + 1] != NULL){
            write (STDERR_FILENO, error_message, strlen(error_message));
        }
    }
    return (redirect);
}

int execute (char ** path, char ** theCmd) {
    int pid;
    int i = 0;
    int pathFound = 0; 
    int success = 1; 
    char thePath[128];
    int redir = 0;
    redir = checkRedirect(theCmd);
    if(redir == 1) {
        redirect(path, theCmd);
        return (redir);   
    } 
    pid = fork();
    if(pid == 0 && redir == 0) {
        while(path[i] != NULL) {
            strcpy(thePath, path[i]);
            strcat(thePath, "/");
            strcat(thePath, theCmd[0]);
                if((execv(thePath, theCmd)) == -1) {
                    success = 0;
                }
                else {
                    success = 1;
                }
            i++;
        }
    }
    else if (pid < 0) {
        write (STDERR_FILENO, error_message, strlen(error_message));
    }
    else {
        if((wait(&pid)) < 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
    return (success); 
}


int main (int argc, char * argv[]) {
    char * line = malloc(128);
    pid_t wpid;
    int pid;
    char * args = malloc(128);
    int i = 0;
    char ** path = malloc(128);
    char ** theCmd;
    char * cwd = malloc(512);  
    int pathFound = 0;
    path[0] = "/bin";
    if (argc > 1) {
        write (STDERR_FILENO, error_message, strlen(error_message));
    }

    while (1) {
        printf ("whoosh> ");
        if (fgets(line, 128, stdin) == NULL) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }  
        for(i = 0; i < 128; i++) {
            if(strstr(line, "\n") == NULL) {
                write (STDERR_FILENO, error_message, strlen(error_message));
            }
        }

        theCmd = tokenize(line);
        if (strcmp (theCmd[0], "cd") == 0) {
            if (theCmd[1] == NULL) { 
                chdir(getenv("HOME"));
            } 

            else {
                if((chdir(theCmd[1])) == -1){
                    write (STDERR_FILENO, error_message, strlen(error_message));
                }
            }      
        }

        else if (strcmp (theCmd[0], "pwd") == 0) {
            getcwd (cwd, 128); 
            if (cwd == NULL) {
                write (STDERR_FILENO, error_message, strlen(error_message));
            }
            else {
                printf("%s\n", cwd);
            }
        }

        else if (strcmp (theCmd[0], "path") == 0) {
            i = 1;
            while (theCmd[i] != NULL) {
                path[i - 1] = malloc(strlen(theCmd[i]) + 1);
                strcpy(path[i - 1], theCmd[i]);
                i++;
                printf("path = %s\n", path[i-2]);
            }      
        }
 
        else if (strcmp (theCmd[0], "exit") == 0) {
            break;    
        } 
        
        else {
            if((execute(path, theCmd)) == 0) {
                write (STDERR_FILENO, error_message, strlen(error_message));
            } 
        }
    }
    exit(0);
}
