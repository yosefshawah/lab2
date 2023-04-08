#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "LineParser.h"
#include <fcntl.h>

#define MAx_INPUT_BUFFER 2048

void printPrompt(){
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        printf("%s$ ", cwd);
    } else {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
}

int execute(cmdLine *pCmdLine) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // child process
          if (strcmp(pCmdLine->arguments[0], "quit") == 0) {
             exit(EXIT_SUCCESS);
         }

        if (pCmdLine->inputRedirect != NULL) {
            int fd = open(pCmdLine->inputRedirect, O_RDONLY);
            if (fd == -1) {
                perror("open input file error");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (pCmdLine->outputRedirect != NULL) {
            int fd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd == -1) {
                perror("open output file error");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execvp error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        // parent process
        if (pCmdLine->blocking) {
            int status;
            if (waitpid(pid, &status, WUNTRACED) == -1) {
                perror("waitpid error");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("PID: %d\n", pid);
        }
        return pid;
    }
}



int main(int argc, char **argv){
    char userInput[MAx_INPUT_BUFFER];

    int debugMode = 0;
    for(int i = 0; i < argc; i++){
        
        if(argc > 1 && strcmp(argv[1], "-d") == 0) {
        debugMode = 1;
        }

    }
    

    while (1){
        printPrompt();
        fgets(userInput, MAx_INPUT_BUFFER, stdin);
        cmdLine *parsedCmdLine = parseCmdLines(userInput);
        if(parsedCmdLine == NULL){
            continue;
        }
        
        execute(parsedCmdLine);

        if(debugMode){
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", parsedCmdLine->arguments[0]);
        }

        if(strcmp(parsedCmdLine->arguments[0], "quit") == 0){
            break;
        }

        if(strcmp(parsedCmdLine->arguments[0], "cd") == 0) {//this needs fix, it prints to stderr when i use quit.
            if (parsedCmdLine->argCount >= 2 && chdir(parsedCmdLine->arguments[1]) == -1) {
                fprintf(stderr, "cd: %s: No such file or directory\n", parsedCmdLine->arguments[1]);
            }
        }

        freeCmdLines(parsedCmdLine);
    }

    exit(EXIT_SUCCESS);
}


/*
Once you execute your program, you will notice a few things:
Although you loop infinitely, the execution ends after execv. Why is that?
You must place the full path of an executable file in-order to run properly. For instance: "ls" won't work, whereas "/bin/ls" runs properly. (Why?)

After executing execv or any of its variants, the process image is replaced with the image of the new program specified in the arguments of the execv function.
In other words, the new program starts running and the original program (in our case, the shell program) stops running.

Regarding the need to use the full path of the executable file, it is because the shell program does not know where to find the executable file
if you just type its name. When you type a command like ls in the shell, the shell looks for the command in a list of directories specified in
the PATH environment variable. If the executable file is not found in any of these directories, the shell will not be able to run it. However,
if you provide the full path of the executable file (e.g., /bin/ls), the shell will know exactly where to find it and run it successfully.

*/

/*
Now replace execv with execvp (see man) and try again .
Wildcards, as in "ls *", are not working. (Again, why?)

The reason why wildcards, such as "*" are not working in the shell is because they are expanded by the shell itself,
before the arguments are passed to the executed program. When using functions like execv or execvp, the shell is not involved in the execution of the program,
so the wildcards are not expanded. Instead, the program receives the wildcard characters as regular characters in its arguments.

To make wildcards work in a program executed by execvp, you can use the glob function (see man) to manually expand the wildcards in the program's arguments.

*/