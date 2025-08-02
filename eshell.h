#ifndef ESHELL_H
#define ESHELL_H

#include "parser.h"

char* quitText = "quit";
// Executes a single command in a child process
void singleCommandExecutor(command* cmd);

// Executes a pipeline of commands (cmd1 | cmd2 | ...)
void pipeExec(pipeline* pline);

// Sequential execution: cmd1 ; cmd2 ; ...
void sequentialInstructionExecutor(int instructionNumber, parsed_input* input, int isPipe);
void sequentialExecutionHandler(parsed_input* input);

// Parallel execution: cmd1 , cmd2 , ...
void parallelInstructionExecutor(int instructionNumber, parsed_input* input, int isPipe);
void parallelExecutionHandler(parsed_input* input);

// Subshell execution: ( ... )
void subshellExecutionHandler(parsed_input* input, int instructionNumber);

// Quit checker
int checkQuit(command* cmd);

#endif // ESHELL_H
