#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

char error_message[30] = "An error has occurred\n";

/* 
 * Takes in a line entered by the user and then parses the line into
 * an array of commands.
 */

char ** tokenize (char * line) {
    char ** theCmd = malloc(128 * sizeof(char*));
    char * token = strtok (line, " \n");
    char * space = " ";
    int i = 0;
    while (token != NULL) {
        theCmd[i] = malloc (strlen(token) + 1);
        strcpy (theCmd[i], token);
	i++;
        token = strtok(NULL, " \n");
    }
    theCmd[i] = NULL;
    if (theCmd[0] == NULL || theCmd[0] == space) {
        theCmd[0] = malloc(10);
        theCmd[0] = "broke";          
    }
    return (theCmd);
}

struct stat fileStat;

/* 
 * Performs redirection by taking in the path to the program to be executed
 * as well as the command itself, used to build the full path as well as
 * search for the name of the file to redirect to.  It will first search the
 * possible paths in path and if the file to write to specified by theCmd[index]
 * is not found it will write the output to the current directory the user is in.
 */
void redirect (char ** path, char ** theCmd) {
    char output[128];
    char error[128];
    char thePath[128];
    int i = 0;
    int pid;
    int index = 0;
    while (theCmd[i] != NULL) {
        if ((strcmp(theCmd[i], ">")) == 0) {
            index = i + 1;
            theCmd[i] = NULL;
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
        if ((stat(thePath, &fileStat)) == -1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else {
            free(theCmd[0]);
            theCmd[0] = malloc(strlen(thePath) + 1);
            theCmd[0] = strdup(thePath);
            break;
        } 
        i++;
    }
    pid = fork();
    if (pid == 0){ 
        if(freopen(output, "w+", stdout) == NULL) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }
        if(freopen(error, "w+", stderr) == NULL) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }
        execv(thePath, theCmd);
        write (STDERR_FILENO, error_message, strlen(error_message));
	_exit(1);
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

/*
 * perfroms all of the necessary checks to make sure an error does not occur when
 * performing redirection.
 *
 * Returns and int that specifies the program can or cannot perform redirection.
 */

int checkRedirect (char ** theCmd) {
    int i = 0;
    int count = 0;
    int index = 0;
    while (theCmd[i] != NULL) {
        if ((strcmp(theCmd[i], ">")) == 0) {
            count++;
            
            index = i + 1;
        }
        i++;
    }
    if (count >= 1) {
        if(theCmd[index] == NULL || theCmd[index + 1] != NULL || count > 1) {
            write (STDERR_FILENO, error_message, strlen(error_message));
            return(-1);
        }
        else {
            return (1);
        }
    }
    return (0);
}

/* 
 * Takes in the path to the program entered which is stored in theCmd and
 * executes the program in a child shell.
 */

void execute (char ** path, char ** theCmd) {
    int pid;
    int i = 0;
    char thePath[128];
    int redir = 0;
    i = 0;
    redir = checkRedirect(theCmd);
    if(redir == 1) {
        redirect(path, theCmd);
        return;   
    } 
    if (redir == -1) {
        return;
    }
    pid = fork();
    if(pid == 0 && redir == 0) {
        while(path[i] != NULL) {
            strcpy(thePath, path[i]);
            strcat(thePath, "/");
            strcat(thePath, theCmd[0]);
            if ((stat(thePath, &fileStat)) == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else {
                free(theCmd[0]);
                theCmd[0] = malloc(strlen(thePath) + 1);
                theCmd[0] = strdup(thePath);
                break;
            }
            i++;
        }
        execv(thePath, theCmd);
        _exit(1);
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

/* 
 * Contains main loop of the program that first checks if the command entered
 * was a built in command, if not then perform execute program and search for
 * the program to execute in the specified path.
 */

int main (int argc, char * argv[]) {
    char * line = malloc(1500);
    int i = 0;
    char ** path = malloc(128);
    char ** theCmd;
    char * cwd = malloc(512);
    path[0] = malloc(30);  
    strcpy(path[0], "/bin");
    if (argc > 1) {
        write (STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    while (1) {
        write (STDOUT_FILENO, "whoosh> ", 8);
        fflush(0);
        
        if (fgets(line, 1500, stdin) == NULL) {
            write (STDERR_FILENO, error_message, strlen(error_message));
            continue;
        } 
        if ((strlen(line)) > 129) {
            write (STDERR_FILENO, error_message, strlen(error_message));
            continue;
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
                strcat(cwd, "\n");
                write(STDOUT_FILENO, cwd, strlen(cwd));
            }
        }

        else if (strcmp (theCmd[0], "path") == 0) {
            i = 1;
            while (theCmd[i] != NULL) {
                path[i - 1] = malloc(strlen(theCmd[i]) + 1);
                strcpy(path[i - 1], theCmd[i]);
                i++;
            }
            if (theCmd[1] == NULL) {
                path[0] = NULL;
            }      
        }
 
        else if (strcmp (theCmd[0], "exit") == 0) {
            break;    
        } 
        
        else {
            if ((strcmp(theCmd[0], "broke")) != 0) {
                execute(path, theCmd); 
            }
        }
    }
    exit(0);
}
