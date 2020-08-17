# LINUX Shell
status: ongoing

## Overview
Implementation of LINUX Shell in C: a program that runs in an infinite loop in which it prompts for user input, execute the command, and returns to the next prompt

**Basic Functionalities:**
- Handling user interrupt signal 
- Command sequences separated by semicolons (;) and ampersands (&)
- Backgrounding commands using the standard (&) notation
- I/O file redirection using (<), (>), and appending (>>)
- Piping between programs using the vertical bar (|) notation
- Execute both internal commands (ls, pwd, cd etc.) and external commands (using execvp system call)


## Compiling
```shell
make
./shell
```

## References



