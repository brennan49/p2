#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

char error_message[30] = "An error has occurred\n";
#define MAX_LENGTH 128;

int main (int argc, char * argv[]) {

    char * cmd;
    char * line = malloc (128);
    pid_t pid, wpid;
    char * args;

    if (argc > 1) {
        write (STDERR_FILENO, error_message, strlen(error_message));
    }

    while (1) {
        printf ("whoosh> ");
        if (fgets(line, 128, stdin) == NULL) {
            write (STDERR_FILENO, error_message, strlen(error_message));
        }
        
        printf("%s", line);
        
    
        if (cmd = strtok (line, " ")) {
            printf("%s", cmd);
            printf("hello\n");
            if (strcmp (cmd, "ls\n") == 0) {
                pid = fork();
                if (pid == 0) {
                    if (execv("/bin/ls", line) == -1);
                        write (STDERR_FILENO, error_message, strlen(error_message)); 
                }      
            }

            else if (pid < 0) {
                write (STDERR_FILENO, error_message, strlen(error_message));
            }

            else {
                waitpid(pid);    
            }
            
            printf("%s", line);
            if (strcmp (cmd, "cd\n") == 0) {
                args = strtok (NULL, " ");
                printf("butts"); 
                if (args == NULL) { 
                    printf("args is null");
                    getenv("HOME");
                } 
                else {
                    chdir(args);
                }      
            }
            
            if (strcmp (cmd, "pwd\n") == 0) {
                
            }

            if (strcmp (cmd, "path\n") == 0) {
                
            }
 
            if (strcmp (cmd, "exit\n") == 0) {
                exit(0);    
            }
            
        }  
    }

}
