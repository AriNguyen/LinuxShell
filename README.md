# LINUX Shell
status: ongoing

## Overview
Implementation of LINUX Shell in C: a program that runs in an infinite loop in which it prompts for user input, execute the command, and returns to the next prompt

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

## Compiling
```shell
make
./shell
```

## References



