#include <unistd.h>
#include <stdio.h>
#include "parser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "eshell.h"


int checkQuit(command* cmd){

    if (strcmp(quitText, cmd->args[0])==0){
        exit(1);
    }
    return 0;
    
}

void pipeExec(pipeline* pline) {
    int n = pline->num_commands;
    int pipes[n-1][2];
    int status;

    // Create pipes
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    // Fork for each command
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) { // child
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);   // read from previous pipe
            }
            if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);    // write to next pipe
            }

            // Close all pipes in child
            for (int j = 0; j < n - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            command *cmd = &pline->commands[i];
            char **arg = cmd->args;

            execvp(arg[0], arg);
            perror("execvp failed");
            exit(1);
        }
    }

    // Parent closes all pipes
    for (int i = 0; i < n - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Parent waits for all children
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
}

void subshellExecutionHandler(parsed_input* input, int instructionNumber){
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) { // child runs subshell
        parsed_input subCom;
        parse_line(input->inputs[instructionNumber].data.subshell, &subCom);

        if (subCom.separator == SEPARATOR_SEQ) {
            sequentialExecutionHandler(&subCom);
        }
        else if (subCom.separator == SEPARATOR_PARA) {
            parallelExecutionHandler(&subCom);
        }
        else if (subCom.separator == SEPARATOR_PIPE) {
            pipeline pl;
            pl.num_commands = subCom.num_inputs;
            for (int i = 0; i < subCom.num_inputs; i++)
                pl.commands[i] = subCom.inputs[i].data.cmd;
            pipeExec(&pl);
        }
        else if (subCom.separator == SEPARATOR_NONE) {
            if (subCom.inputs[0].type == INPUT_TYPE_SUBSHELL) {
                // recursion for nested subshells
                subshellExecutionHandler(&subCom, 0);
            } else {
                singleCommandExecutor(&(subCom.inputs[0].data.cmd));
            }
        }

        free_parsed_input(&subCom);
        exit(0); // child exits after subshell
    }
    else { // parent waits
        int status;
        waitpid(pid, &status, 0);
    }
}

void sequentialInstructionExecutor(int instructionNumber, parsed_input* input, int isPipe){
    int childStatus = 0;
    pid_t childPID = fork();
    if (childPID){
        waitpid(childPID, &childStatus, 0);
    }
    else{
        if(isPipe == 0){ // command
            command* cmd = &(input->inputs[instructionNumber].data.cmd);
            execvp(cmd->args[0], cmd->args);
        }
        else if (isPipe == 1){ // pipeline
            pipeline* pline = &(input->inputs[instructionNumber].data.pline);
            pipeExec(pline);
        }

    }

}
void sequentialExecutionHandler(parsed_input *input)
{
    for (int i = 0; i < input->num_inputs; i++)
    {

        if (input->inputs[i].type == INPUT_TYPE_COMMAND)
        {

            sequentialInstructionExecutor(i, input, 0);
        }

        if (input->inputs[i].type == INPUT_TYPE_PIPELINE)
        {
            sequentialInstructionExecutor(i, input, 1);
        }
    }
}

void parallelInstructionExecutor(int instructionNumber, parsed_input *input, int isPipe)
{   int childStatus = 0;
    pid_t childPID = fork();

    if (childPID){ // parent
        waitpid(childPID, &childStatus, 0);
        return;
    }
    else{ //child
        if (isPipe == 0){
            command *cmd = &(input->inputs[instructionNumber].data.cmd);
            execvp(cmd->args[0], cmd->args);
        }
        else if (isPipe == 1){
            pipeline* pline = &(input->inputs[instructionNumber].data.pline);
            pipeExec(pline);
        }
    }
    
}

void parallelExecutionHandler(parsed_input *input)
{
    pid_t pids[MAX_INPUTS];
    int childCount = 0;

    // Launch all instructions
    for (int i = 0; i < input->num_inputs; i++)
    {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) { // Child
            if (input->inputs[i].type == INPUT_TYPE_COMMAND) {
                command *cmd = &(input->inputs[i].data.cmd);
                execvp(cmd->args[0], cmd->args);
                perror("execvp failed");
                exit(1);
            }
            else if (input->inputs[i].type == INPUT_TYPE_PIPELINE) {
                pipeline* pline = &(input->inputs[i].data.pline);
                pipeExec(pline);
                exit(0);
            }
        }
        else { // Parent
            pids[childCount++] = pid;
        }
    }

    // Parent waits for all children
    for (int j = 0; j < childCount; j++) {
        waitpid(pids[j], NULL, 0);
    }
}

void singleCommandExecutor(command* cmd){
    int childStatus;
    pid_t childPID = fork();

    if (childPID)
    {
        waitpid(childPID, &childStatus, 0);
    }
    else //child
    {
        execvp(cmd->args[0], cmd->args);
    }
}

int main(void) {

    while (1) {
        parsed_input input;
        char lineBuffer[256];
        printf("/> ");
        fgets(lineBuffer, 256, stdin);
        int parsedLine = parse_line(lineBuffer, &input);
        if (!parsedLine){
            free_parsed_input(&input);
            continue;
        }


        if (input.separator == SEPARATOR_NONE){
            if (input.inputs[0].type == INPUT_TYPE_SUBSHELL){

                subshellExecutionHandler(&input, 0);

            }
            else if (input.inputs[0].type == INPUT_TYPE_COMMAND){
                command* cmd = &(input.inputs[0].data.cmd);
                singleCommandExecutor(cmd);
            }
        }
        else if (input.separator == SEPARATOR_PIPE)
        {
            pipeline pl;
            pl.num_commands = input.num_inputs;
            for (int i = 0; i < input.num_inputs; i++) {
                pl.commands[i] = input.inputs[i].data.cmd;
            }
            pipeExec(&pl);

        }

        else if (input.separator == SEPARATOR_PARA)
        {
            parallelExecutionHandler(&input);
        }

        else if (input.separator == SEPARATOR_SEQ)
        {
            sequentialExecutionHandler(&input);
        }

        free_parsed_input(&input);
    }
    
    return 0;
}
