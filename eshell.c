#include <unistd.h>
#include <stdio.h>
#include "parser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void seqExec(parsed_input* input);
void pipeExec(parsed_input* input);
void seqInsExec(int a, parsed_input* input, int isPipe);
void seqExec(parsed_input* input);
void parInsExec(int a, parsed_input* input, int isPipe);
void paraExec(parsed_input* input);

int checkQuit(parsed_input* input){
    single_input *inp = &input->inputs[0];
    command* cmd = &inp->data.cmd;
    
    char* q = "quit";

    if (strcmp(q, (*cmd).args[0])==0)
    { 
       
        exit(1);
        
    }
    return 0;
    
}

void subExec(parsed_input* input, int a){
	if (input->inputs[a].type==INPUT_TYPE_SUBSHELL){
                parsed_input subCom;
                parse_line(input->inputs[a].data.subshell, &subCom);
                
                if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                }

                else if (subCom.separator==SEPARATOR_SEQ){
                    seqExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_NONE){
                    	int childStat;
                    int forkVal;
                    
                    if (fork()){ 

                        wait(&childStat);
                        free_parsed_input(&subCom);
                        
                    }
                    else {
                        
                        single_input inp = subCom.inputs[0];
                        command cm = inp.data.cmd;

                        
                        if(!execv(cm.args[0], cm.args)){
                            printf("hatalı döndü");
                            
                        }
                        else {
                            execvp(cm.args[0], cm.args);
                        }
                        
                    
                    }
                }

                else if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_PIPE){
            
            
                    pipeExec(&subCom);
                    free_parsed_input(&subCom);

                }
                exit(1);

            }
}

void pipeExec(parsed_input* input){

    int pipes[10][2];
    pipe(pipes[0]);
    int cS;
    
    
    if (!fork()){ //first child runs this block gives me subshell
        dup2(pipes[0][1],1);
        close(pipes[0][1]);
        if (input->inputs[0].type==INPUT_TYPE_COMMAND) {
		single_input *inp = &input->inputs[0];
		command *cmd = &inp->data.pline.commands[0];
		char **arg = cmd->args;
		if(!execv(*arg, arg)){
		    printf("hatalı döndü");
		}
		else {
		    execvp(*arg, arg);
		}
        }
        
        if (input->inputs[0].type==INPUT_TYPE_SUBSHELL){
                parsed_input subCom;
                parse_line(input->inputs[0].data.subshell, &subCom);
                
                 /*if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                }*/

                /*else*/ if (subCom.separator==SEPARATOR_SEQ){
                    seqExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_NONE){
                    	int childStat;
                    int forkVal;
                    
                    if (fork()){ 

                        wait(&childStat);
                        free_parsed_input(&subCom);
                        
                    }
                    else {
                        
                        single_input inp = subCom.inputs[0];
                        command cm = inp.data.cmd;

                        
                        if(!execv(cm.args[0], cm.args)){
                            printf("hatalı döndü");
                            
                        }
                        else {
                            execvp(cm.args[0], cm.args);
                        }
                        
                    
                    }
                }

                else if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                     
                    
                }
                else if (subCom.separator==SEPARATOR_PIPE){
            
            
                    pipeExec(&subCom);
                    free_parsed_input(&subCom);

                }
                exit(1);

            }
            

    }
    else {
        close(pipes[0][1]);
    }

    for (int i=1;i<input->num_inputs-1;i++){

        pipe(pipes[i]);
        if (!fork()){ //middle children run this block
            dup2(pipes[i-1][0],0);
            close(pipes[i-1][0]);
            dup2(pipes[i][1],1);
            close(pipes[i][1]);
            if (input->inputs[i].type==INPUT_TYPE_COMMAND) {
		    single_input *inp = &input->inputs[i];
		    command *cmd = &inp->data.pline.commands[0];
		    char **arg = cmd->args;
		    if(!execv(*arg, arg)){
		        printf("hatalı döndü");
		    }
		    else {
		        execvp(*arg, arg);
		    }
            }
            if (input->inputs[i].type==INPUT_TYPE_SUBSHELL){
                parsed_input subCom;
                parse_line(input->inputs[i].data.subshell, &subCom);
                
                if (subCom.separator==SEPARATOR_PARA){
                    
                    int pipeSubPar[10][2];
                    char buf[300000];
                    for (int i=0;i<subCom.num_inputs;i++){
                    	pipe(pipeSubPar[i]);
                    	if(!fork()){//child not repeater
                    		dup2(pipeSubPar[i][0],0);
                    		close(pipeSubPar[i][0]);
                    		close(pipeSubPar[i][1]);
				//read(STDIN_FILENO, buf, 300000);
				//printf("%s",buf);
                    		single_input inp = subCom.inputs[i];
		                command cm = inp.data.cmd;
				//exit(1);
		                if(!execv(cm.args[0], cm.args)){
		                    printf("hatalı döndü");
		                    
		                }
		                else {
		                    execvp(cm.args[0], cm.args);
		                }
                    	}
                    	else{ //subshell runs this
                    		close(pipeSubPar[i][0]);
                    		
                    	
                    	}
                    	
                    }
                    if (!fork()){ // repeater runs this
                    	signal(SIGPIPE, SIG_IGN);
                    	ssize_t n;
                    	char buf[256*1024];
                    	while ((n=read(STDIN_FILENO, buf, 1))>0){
		            	for (int i=0;i<subCom.num_inputs;i++){
		            		
		            		dup2(pipeSubPar[i][1],STDOUT_FILENO); // write
		            		printf("%s",buf);
		            		fflush(stdout);
		            	}
                    	}
                    	//read(STDIN_FILENO, buf, 256*1024);
                    	int cS;
                    		
                    	
                    	exit(1);
                    	
                    	
                    	
                    	
                    }
                    else {//subshell runs this
                    int cS;
                    	for (int i=0;i<subCom.num_inputs;i++){
                    		close(pipeSubPar[i][1]);
                    	}
                    	for (int i=0;i<subCom.num_inputs+1;i++){
                    		wait(&cS);
                    	}
                    }
                }

                else if (subCom.separator==SEPARATOR_SEQ){
                    seqExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_NONE){
                    	int childStat;
                    int forkVal;
                    
                    if (fork()){ 

                        wait(&childStat);
                        free_parsed_input(&subCom);
                        
                    }
                    else {
                        
                        single_input inp = subCom.inputs[0];
                        command cm = inp.data.cmd;

                        
                        if(!execv(cm.args[0], cm.args)){
                            printf("hatalı döndü");
                            
                        }
                        else {
                            execvp(cm.args[0], cm.args);
                        }
                        
                    
                    }
                }

                else if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_PIPE){
            
            
                    pipeExec(&subCom);
                    free_parsed_input(&subCom);

                }
                exit(1);

            }
        }
        else {
            close(pipes[i][i]);
            close(pipes[i-1][i-1]);
        }
    }
    

    if (!fork()){ //last child runs this block
        dup2(pipes[input->num_inputs-2][0],0);
        close(pipes[input->num_inputs-2][0]);
        if (input->inputs[input->num_inputs-1].type==INPUT_TYPE_COMMAND) {
		single_input *inp = &input->inputs[input->num_inputs-1];
		command *cmd = &inp->data.pline.commands[0];
		char **arg = cmd->args;
		if(!execv(*arg, arg)){
		    printf("hatalı döndü");
		}
		else {
		    if (!execvp(*arg, arg)){
		        printf("hatalı döndüq");}
		    else {
		        printf("%s",*arg);
		        }
		}
        }
        if (input->inputs[input->num_inputs-1].type==INPUT_TYPE_SUBSHELL){
                parsed_input subCom;
                parse_line(input->inputs[input->num_inputs-1].data.subshell, &subCom);
                
                /*if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                }*/

                /*else*/ if (subCom.separator==SEPARATOR_SEQ){
                    seqExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_NONE){
                    	int childStat;
                    int forkVal;
                    
                    if (fork()){ 

                        wait(&childStat);
                        free_parsed_input(&subCom);
                        
                    }
                    else {
                        
                        single_input inp = subCom.inputs[0];
                        command cm = inp.data.cmd;

                        
                        if(!execv(cm.args[0], cm.args)){
                            printf("hatalı döndü");
                            
                        }
                        else {
                            execvp(cm.args[0], cm.args);
                        }
                        
                    
                    }
                }

                else if (subCom.separator==SEPARATOR_PARA){
                    int pipeSubPar[10][2];
                    
                    for (int i=0;i<subCom.num_inputs;i++){
                    	pipe(pipeSubPar[i]);
                    	if(!fork()){//child not repeater
                    		dup2(pipeSubPar[i][0],0);
                    		close(pipeSubPar[i][0]);
                    		close(pipeSubPar[i][1]);
				//read(STDIN_FILENO, buf, 300000);
				//printf("%s",buf);
                    		single_input inp = subCom.inputs[i];
		                command cm = inp.data.cmd;
				//exit(1);
		                if(!execv(cm.args[0], cm.args)){
		                    printf("hatalı döndü");
		                    
		                }
		                else {
		                    execvp(cm.args[0], cm.args);
		                }
                    	}
                    	else{ //subshell runs this
                    		close(pipeSubPar[i][0]);
                    		
                    	
                    	}
                    	
                    }
                    if (!fork()){ // repeater runs this
                    	signal(SIGPIPE, SIG_IGN);
                    	ssize_t n;
                    	char buf[256*1024];
                    	while ((n=read(STDIN_FILENO, buf, 1))>0){
		            	for (int i=0;i<subCom.num_inputs;i++){
		            		
		            		dup2(pipeSubPar[i][1],STDOUT_FILENO); // write
		            		printf("%s",buf);
		            		fflush(stdout);
		            	}
                    	}
                    	//read(STDIN_FILENO, buf, 256*1024);
                    	int cS;
                    		
                    	
                    	exit(1);
                    	
                    	
                    	
                    	
                    }
                    else {//subshell runs this
                    int cS;
                    	for (int i=0;i<subCom.num_inputs;i++){
                    		close(pipeSubPar[i][1]);
                    	}
                    	for (int i=0;i<subCom.num_inputs+1;i++){
                    		wait(&cS);
                    	}
                    }
                    
                }
                
                else if (subCom.separator==SEPARATOR_PIPE){
            
            
                    pipeExec(&subCom);
                    free_parsed_input(&subCom);

                }
                exit(1);

            }
        
    }
    else {
        close(pipes[input->num_inputs-2][0]);
        
        for (int i=0;i<input->num_inputs;i++){
            wait(&cS);
        }
    }
 

}


void seqInsExec(int a, parsed_input* input, int isPipe){
    
    if(!isPipe){
        int cS;
        
        if (!fork()){

            single_input *inp = &input->inputs[a];
                    command *cmd = &inp->data.pline.commands[0];
                    char **arg = cmd->args;
                    
                    if(!execv(*arg, arg)){
                        printf("hatalı döndü");
                    }
                    else {
                        execvp(*arg, arg);
                    }
        }
        else {
            wait(&cS);
        }
                    
    }
    
    else{
        int pipesS[10][2];
        pipe(pipesS[0]);
        int cS;
        single_input *inp = &input->inputs[a];
        int numOfIns = inp->data.pline.num_commands;
        
        
        if (!fork()){ //first child runs this block
            dup2(pipesS[0][1],1);
            close(pipesS[0][1]);
            command *cmd = &inp->data.pline.commands[0];
            char **arg = cmd->args;
            if(!execv(*arg, arg)){
                printf("hatalı döndü");
            }
            else {
                execvp(*arg, arg);
            }
        }
        else {
            close(pipesS[0][1]);
        }

        for (int i=1;i<numOfIns-1;i++){

            pipe(pipesS[i]);
            if (!fork()){ //middle children run this block
                dup2(pipesS[i-1][0],0);
                close(pipesS[i-1][0]);
                dup2(pipesS[i][1],1);
                close(pipesS[i][1]);
                
                command *cmd = &inp->data.pline.commands[i];
                char **arg = cmd->args;
                if(!execv(*arg, arg)){
                    printf("hatalı döndü");
                }
                else {
                    execvp(*arg, arg);
                }
            }
            else {
                close(pipesS[i][i]);
                close(pipesS[i-1][i-1]);
            }
        }
    

        if (!fork()){ //last child runs this block
            dup2(pipesS[numOfIns-2][0],0);
            close(pipesS[numOfIns-2][0]);
            command *cmd = &inp->data.pline.commands[numOfIns-1];
            char **arg = cmd->args;
            if(!execv(*arg, arg)){
                printf("hatalı döndü");
            }
            else {
                execvp(*arg, arg);
            }
        }
        else {
            close(pipesS[numOfIns-2][0]);
            
            for (int i=0;i<numOfIns;i++){
                wait(&cS);
            }
        }
    }

}
void seqExec(parsed_input* input){
    int mixed = 0;
 
        int cS;
        for (int i=0;i<input->num_inputs;i++){
            
            if (input->inputs[i].type==INPUT_TYPE_COMMAND){
                
                seqInsExec(i, input, 0);     

            }

            if (input->inputs[i].type==INPUT_TYPE_PIPELINE){
                seqInsExec(i, input, 1);
            }

        }
    
}

void parInsExec(int a, parsed_input* input, int isPipe) {
    if(!isPipe){
        int cS;
        if (!fork()){

            single_input *inp = &input->inputs[a];
                    command *cmd = &inp->data.pline.commands[0];
                    char **arg = cmd->args;
                    
                    if(!execv(*arg, arg)){
                        printf("hatalı döndü");
                    }
                    else {
                        execvp(*arg, arg);
                    }
        }
                    
    }
    
    else{

        int cS;
        single_input *inp = &input->inputs[a];
        int numOfIns = inp->data.pline.num_commands;
        
        
        if (!fork()){ //first child runs this block

            command *cmd = &inp->data.pline.commands[0];
            char **arg = cmd->args;
            if(!execv(*arg, arg)){
                printf("hatalı döndü");
            }
            else {
                execvp(*arg, arg);
            }
        }

        for (int i=1;i<numOfIns-1;i++){

            if (!fork()){ //middle children run this block

                command *cmd = &inp->data.pline.commands[i];
                char **arg = cmd->args;
                if(!execv(*arg, arg)){
                    printf("hatalı döndü");
                }
                else {
                    execvp(*arg, arg);
                }
            }

        }
    

        if (!fork()){ //last child runs this block

            command *cmd = &inp->data.pline.commands[numOfIns-1];
            char **arg = cmd->args;
            if(!execv(*arg, arg)){
                printf("hatalı döndü");
            }
            else {
                execvp(*arg, arg);
            }
        }

}

}

void paraExec(parsed_input* input) {
        int cS;
        for (int i=0;i<input->num_inputs;i++){
            
            if (input->inputs[i].type==INPUT_TYPE_COMMAND){
                
                parInsExec(i, input, 0);     

            }

            if (input->inputs[i].type==INPUT_TYPE_PIPELINE){
                parInsExec(i, input, 1);
            }
            
            

        }
        for (int i=0;i<input->num_inputs;i++){
            
            wait(&cS);

        }
}

int main(void) {

    while ( 1==1 ) {
        
        parsed_input input;
        char s[256];
        printf("/> ");
        fgets(s, 256, stdin);
        int valid = parse_line(s, &input);
        
        if (valid){
            
            
            //checkQuit(&input);
            //printf("%s\n", s);
            
            if (input.separator==SEPARATOR_NONE && input.inputs[0].type==INPUT_TYPE_SUBSHELL){
                parsed_input subCom;
                parse_line(input.inputs->data.subshell, &subCom);
                
                if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                }

                else if (subCom.separator==SEPARATOR_SEQ){
                    seqExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_NONE){
                    	int childStat;
                    int forkVal;
                    
                    if (fork()){ 

                        wait(&childStat);
                        free_parsed_input(&input);
                        
                    }
                    else {
                        
                        single_input inp = subCom.inputs[0];
                        command cm = inp.data.cmd;

                        
                        if(!execv(cm.args[0], cm.args)){
                            printf("hatalı döndü");
                            
                        }
                        else {
                            execvp(cm.args[0], cm.args);
                        }
                        
                    
                    }
                }

                else if (subCom.separator==SEPARATOR_PARA){
                    paraExec(&subCom);
                    free_parsed_input(&subCom);
                }
                else if (subCom.separator==SEPARATOR_PIPE){
            
            
                    pipeExec(&subCom);
                    free_parsed_input(&subCom);

                }

            }
            
            else if (input.separator==SEPARATOR_NONE && input.inputs[0].type==INPUT_TYPE_COMMAND) {
                
                checkQuit(&input);
                if (input.inputs[0].type==INPUT_TYPE_COMMAND)
                    {int childStat;
                    int forkVal;
                    
                    if (fork()){ 

                        wait(&childStat);
                        free_parsed_input(&input);
                        
                    }
                    else {
                        
                        single_input inp = input.inputs[0];
                        command cm = inp.data.cmd;

                        
                        if(!execv(cm.args[0], cm.args)){
                            printf("hatalı döndü");
                            
                        }
                        else {
                            execvp(cm.args[0], cm.args);
                        }
                        
                    }
                    }
                

            }

            else if (input.separator==SEPARATOR_PIPE){
            
            
		    pipeExec(&input);
		    free_parsed_input(&input);

            }

            else if (input.separator==SEPARATOR_PARA){
                paraExec(&input);
                free_parsed_input(&input);
            }

            else if (input.separator==SEPARATOR_SEQ){
                seqExec(&input);
                free_parsed_input(&input);
            }

            
        }
        

    }
    
    

    return 0;
}
