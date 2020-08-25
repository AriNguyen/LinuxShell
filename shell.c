#include <stdio.h>  // printf() stdin 
#include <stdlib.h> // exit() 
#include <unistd.h> // fork() chdir() 
#include <string.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/wait.h> 

#define TOKENSIZE 32
#define BUFSIZE 128

int backgrndFlag;
int isAppend;
int isContinue;

void runCommand(char *[], char*, char*, int*, int *);
void tokenizeBySymbol(char *, char *[], char *, int *);


int main(int argc, char *argv[]) {
    int i, j;
    int status, flag;
    int sequenceSize, cmdSize;
    int oldPipe[2], newPipe[2];
    
    char buf[BUFSIZE], *cmd;
    char *sequenceList[512], *tokens[TOKENSIZE], *tokensPipe[TOKENSIZE], char *cmdTokens[TOKENSIZE];
    char *inFile, *outFile;

    pid_t chPid;

    while (1) {
        flag = 0; 
        backgrndFlag = 1;
        isContinue = 1;

        // prompt & get input from stdin 
        write(1, "$ ", 2);
        if (fgets(buf, BUFSIZE, stdin) == NULL) {
            perror("fget");
            exit(1);
        }
		if (!(strcmp(buf, "\n") && strcmp(buf, "")))
			continue;
        if (!(strncmp(buf, "exit", 4) && strncmp(buf, "quit", 4))) {
            flag = 1;
            printf("exit\n");
			break;
		}

        // tokenize to sequences
        if (strchr(buf, ';') != NULL)
            tokenizeBySymbol(buf, sequenceList, ";", &sequenceSize);
        else if (strstr(buf, "&&") != NULL) {
            tokenizeBySymbol(buf, sequenceList, "&&", &sequenceSize);
        } else if (strchr(buf, '&') != NULL) {
            backgrndFlag = 0;
            tokenizeBySymbol(buf, sequenceList, "&", &sequenceSize);
        } else 
            tokenizeBySymbol(buf, sequenceList, "\n", &sequenceSize);

        // execute each sequence
        for (i = 0; i < sequenceSize; i++) {
            cmd = sequenceList[i];
            
            chPid = fork();
            if (chPid < 0) {
                perror("fork");
                exit(2);
            }
            if (chPid == 0) {
                // child process
                inFile = NULL;
                outFile = NULL;
                oldPipe[0] = -1;
                newPipe[0] = -1;
                isAppend = 0;
                if (!isContinue) {
                    break;
                }
                if (strchr(cmd, '|') != NULL) { 
                    tokenizeBySymbol(cmd, tokensPipe, "|" , &cmdSize);
                    for (j = 0; j < cmdSize; j++) {
                        int size;
                        tokenizeBySymbol(tokensPipe[j], cmdTokens, " \n" , &size);
                        pipe(newPipe);
                        if (j == 0) { // first
                            oldPipe[0] = 1;
                            oldPipe[1] = 0;
                        } else if (j == cmdSize - 1) { //last
                            newPipe[0] = 1;
                            newPipe[1] = 0;
                        }
                        runCommand(cmdTokens, inFile, outFile, oldPipe, newPipe);
                        oldPipe[0] = newPipe[0];
                        oldPipe[1] = newPipe[1];
                    }
                } else if (strchr(cmd, '<') != NULL) { 
                    tokenizeBySymbol(cmd, cmdTokens, "<", &cmdSize);
                    inFile = cmdTokens[1];
                    cmdTokens[1] = "\0";
                } else if (strstr(cmd, ">>") != NULL) { 
                    isAppend = 1;
                    tokenizeBySymbol(cmd, cmdTokens, ">>", &cmdSize);
                    outFile = cmdTokens[1];
                    cmdTokens[1] = "\0";
                } else if (strchr(cmd, '>') != NULL) { 
                    tokenizeBySymbol(cmd, cmdTokens, ">", &cmdSize);
                    outFile = cmdTokens[1];
                    cmdTokens[1] = "\0";
                } 
                
                tokenizeBySymbol(cmd, cmdTokens, " \t\n", &cmdSize);
                if (oldPipe[0] < 0)
                    runCommand(cmdTokens, inFile, outFile, oldPipe, newPipe);
            } else { 
                // parent process
                if (newPipe[0] > 1) {
                    close(newPipe[0]);
                    close(newPipe[1]);
                }
                if (backgrndFlag)
                    waitpid(chPid, &status, 0);
            }
        }
        if (flag) {
            printf("\nClosing Shell...\n");
            break;
        }
    }
    exit(0);
}

void runCommand(char *tokens[], char *inFile, char *outFile, int* oldPipe, int* newPipe) {
    int status, inFd, outFd;
    pid_t p1, p2;  

    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork\n"); 
        return; 
    } 
    if (p1 == 0) { 
        // Child 1 executing  
        if (oldPipe[0] > 1) { 
            close(oldPipe[1]); 
            dup2(oldPipe[0], STDIN_FILENO); // direct to write end
            close(oldPipe[0]); // close write
        }
        if (newPipe[0] > 1) { 
            close(newPipe[0]); 
            dup2(newPipe[1], STDOUT_FILENO); // direct to write end
            close(newPipe[1]); // close write
        }
        // redirect to output file
        if (outFile) {
            if (isAppend)
                outFd = open(outFile, O_CREAT| O_RDWR| O_APPEND);
            else
                outFd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC);
            dup2(outFd, STDOUT_FILENO);
            close(outFd);
        }
        // redirect to input file
        if (inFile) {
            inFd = open(inFile, O_RDONLY);
            dup2(inFd, STDIN_FILENO);
            close(inFd);
        }
        // cd cmd
        if (!strcmp(tokens[0], "cd")) {
            chdir(tokens[1]);
        } else {
            execvp(tokens[0], tokens);
            _exit(127);
        }
    } else { 
        // parent executing, closing pipe, waiting for child 1
        if (oldPipe[0] > 1) {
            close(oldPipe[0]);
            close(oldPipe[1]);
        }
        waitpid(p1, &status, 0);
        if (WEXITSTATUS(status) == 127) {
            isContinue = 0;
        }
    }
} 

void tokenizeBySymbol(char *buf, char *cmdExec[], char *symbol, int *sequenceSize) {
    int i = 1;
    cmdExec[0] = strtok(buf, symbol);
    while ((cmdExec[i] = strtok(NULL, symbol)) != NULL)
        i++;
    cmdExec[i] = NULL;
    *sequenceSize = i;
}
