# LINUX Shell
status: ongoing

## Overview
Implementation of LINUX Shell in C: a program that runs in an infinite loop in which it prompts for user input, execute the command, and returns to the next prompt

## Compiling
```shell
make
./shell
```

**Basic Functionalities:**
- Execute internal commands (ls, pwd, cd etc.) and external commands (using execvp system call)
- Command sequences separated by semicolons (;) and ampersands (&)
- Backgrounding commands using the standard (&) notation
- I/O file redirection using (<), (>), and appending (>>)
- Piping between programs using the vertical bar (|) notation

**Working on:**
- Handling user interrupt signal (SIGINT)
- Handling User flag
- Displaying (CWD) to the shell
- Name globbing
- Control Flow
- Handling environment variables
- History

## Pseudocode 
```
FUNCTION main 
    WHILE True 
        Print current working directory CWD 
        Get line from stdin
        Strip newline charater "\n" from the line
        sequences = Parse line by separators (;) or (&) or (&&) inorder from left to right
        FOR seq in sequences 
            commands = Parse seq by pipe (|)
            FOR cmd in commands
                handle IO redirection (<), (>), (>>)
                call: exec command 
            ENDFOR
        ENDFOR
    ENDWHILE
ENDFUNCTION


FUNCTION exec command (arguments: cmd, inFile, outFile, oldPipeFd, newPipeFd)
    pid = fork()
    tokens = Parse cmd by white space " "
    IF in child process
        IF inFile != NULL
            inFd = open(inFile, READ_ONLY_FLAG)
            dup(inFd, STDIN_FILENO)
        ENDIF
        IF outFile != NULL
            outFd = open(outFile)
            dup2(outFd, STDOUT_FILENO)
        ENDIF
        IF oldPipeFd[0] > 1
            close(oldFd[1]);    
            dup2(oldFd[0], 0);  
            close(oldFd[0]);
        ENDIF
        IF newPipeFd[0] > 1
            close(newFd[0]);    
            dup2(newFd[1], 1);  
            close(newFd[1]);
        ENDIF
        IF tokens[0] == "cd"
            chdir(tokens[1])
        ELSE
            execvp(tokens[0], tokens)
        ENDIF
    ELSE IF in parent process
        IF oldPipeFd[0] > 1
            close(oldFd[0]) 
            close(oldFd[1])
        ENDIF
        IF background flag is True
            wait();
        ENDIF
    ENDIF
ENDFUNCTION
```

## Test case 
```shell
cd test
pwd
ls ; pwd ; cd .. ; pwd
ls ; cat ; wc
echo foobar
echo 1 2 3 4 5
./test/out
./test/out &
./test/out ; echo bar
./test/out & echo bar
ls -l | wc -l
./test/out < shell.c
echo bar > bar
echo foobar >> bar
./test/out < foo > baz
./test/out & ./test/out & echo bar
echo 1 ; echo 2 ; echo 3
./test/out < foo ; echo hello > world ; echo goodbye >> world
./test/out < foo | head -n1
./test/out < foo | tail -n1 > baz
./test/out < foo | tail -n1 > baz ; cat baz
cat shell.c | head -n2 | tail -n1
```

## References
