#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define sizeOf(arr) (sizeof(arr) / sizeof(arr[0]))

enum {
    MAXLINE = 512,
    MAXTOKENS = 32
};

int bgFlag;
int isAppend;

void handleSeq(char*);
void handlePipe(char *buf, int);
void handleRedirIO(char*, char*[]); 
void execCommand(char*, char*, char*, int[], int[]);
void tokenizeBySymbol(char*, char**, char*, int*);
void trimWhiteSpace(char*);

int main() {
    char buf[MAXLINE], cwd[PATH_MAX];
    while (1) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return 1;
        }
        printf("%s", strcat(cwd, "\n$ "));
        if (fgets(buf, MAXLINE, stdin) == NULL) {
            break;
        }
        handleSeq(buf);
        printf("\n");
    }
}

void handleSeq(char *buf) {
    int i, index;
    char *p, *bufPtr;
    char seq[MAXTOKENS], sep[MAXTOKENS];
    char *cmdSep[3] = {";", "&", "&&"};

    strtok(buf, "\n");
    isAppend = 0;
    bufPtr = buf;
    while (bufPtr != NULL) {
        index = -1;
        bgFlag = 1;
        for (i = 0; i < sizeOf(cmdSep); i++) {
            if ((p = strstr(bufPtr, cmdSep[i])) != NULL) {
                if (index == -1 || (p - bufPtr) < index) {
                    index = p - bufPtr;
                    memset(seq, 0, MAXTOKENS);
                    strcpy(sep, cmdSep[i]);
                    strncpy(seq, bufPtr, index);
                    bufPtr = ++p;
                }
            }
        }
        if (strcmp(sep, "&") == 0) 
            bgFlag = 0;
        if (index == -1) {
            if (strlen(bufPtr) != 0) {
                strcpy(seq, bufPtr);
                bufPtr = NULL;
            }
            else
                break;
        }
        handlePipe(seq, bgFlag);            
    }
}

void handlePipe(char *buf, int bgFlag) {
    int i, pipeSize;
    int oldFd[2], newFd[2];
    char *pipeTokens[MAXTOKENS];

    if (strchr(buf, '|') != NULL) 
        tokenizeBySymbol(buf, pipeTokens, "|", &pipeSize);
    else {
        pipeTokens[0] = buf;
        pipeSize = 1;
    }
    for (i = 0; i < pipeSize; i++) {
        char *ioFiles[2] = {NULL, NULL};
        if (pipeSize > 1) {
            if (i == 0 && pipeSize > 1) {
                oldFd[0] = 0;
                oldFd[1] = 1;
                pipe(newFd);
            }
            else if (i == pipeSize - 1) {   
                newFd[0] = 0;
                newFd[1] = 1;
            }
            else 
                pipe(newFd);
        }
        handleRedirIO(pipeTokens[i], ioFiles);
        execCommand(pipeTokens[i], ioFiles[0], ioFiles[1], oldFd, newFd);
        oldFd[0] = newFd[0]; 
        oldFd[1] = newFd[1];
    }
}

void handleRedirIO(char *pipe, char *ioFiles[2]) {
    int i;
    char *cmdTokens[MAXTOKENS];
    char *ioSep[] = {">>", ">", "<"};

    for (i = 0; i < sizeOf(ioSep); i++) {
        if (strstr(pipe, ioSep[i]) != NULL) { 
            tokenizeBySymbol(pipe, cmdTokens, ioSep[i], NULL);
            if (strcmp(ioSep[i], ">>") == 0) 
                isAppend = 1;
            if (strcmp(ioSep[i], "<") == 0) 
                ioFiles[0] = cmdTokens[1]; 
            else 
                ioFiles[1] = cmdTokens[1]; 
        }
    }
}

void execCommand(char *cmd, char *inFile, char *outFile, int oldFd[], int newFd[]) {
    int status, flags, inFd, outFd;
    char *tokens[MAXTOKENS];
    pid_t cpid;

    tokenizeBySymbol(cmd, tokens, " ", NULL);
    if ((cpid = fork()) < 0) {
        perror("fork");
        exit(2);
    }
    if (cpid == 0) {
        if (inFile) {
            if ((inFd = open(inFile, O_RDONLY)) < 0) {
                printf("open: no such file or directory: %s\n", inFile);
                return;
            }
            dup2(inFd, 0);
            close(inFd);
        }
        if (outFile) {
            if (isAppend) 
                flags = O_WRONLY | O_APPEND | O_CREAT;
            else
                flags = O_WRONLY | O_CREAT | O_TRUNC;
            if ((outFd = open(outFile, flags)) < 0) {
                printf("open: coudln't open %s\n", outFile);
                return;
            }
            dup2(outFd, 1);
            close(outFd);
        }
        if (oldFd[0] > 1) {
            close(oldFd[1]);    
            dup2(oldFd[0], 0);  
            close(oldFd[0]);
        }
        if (newFd[0] > 1) {
            close(newFd[0]);    
            dup2(newFd[1], 1);  
            close(newFd[1]);
        }
        if (strcmp(tokens[0], "cd") == 0) {
            if (chdir(tokens[1]) != 0)  {
                printf("cd: no such file or directory: %s\n", tokens[1]);
                return;
            }
        }
        else 
            execvp(tokens[0], tokens);
    }
    else {
        if (oldFd[0] > 1) {
            close(oldFd[0]); 
            close(oldFd[1]); 
        }
        if (bgFlag)
            waitpid(cpid, &status, 0);
    }
}

void tokenizeBySymbol(char *str, char **tokens, char *symbol, int *size) {
    int i = 1;
    tokens[0] = strtok(str, symbol);
    trimWhiteSpace(tokens[0]);
    while ((tokens[i] = strtok(NULL, symbol)) != NULL) {
        i++;
        trimWhiteSpace(tokens[1]);
    }
    tokens[i] = NULL;
    if (size) 
        *size = i;
}

void trimWhiteSpace(char *s) {
    char *p = s;
    int l = strlen(p);
    while (isspace(p[l - 1])) 
        p[--l] = 0;
    while (* p && isspace(* p)) 
        ++p, --l;
    memmove(s, p, l + 1);
}