#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_LINE 80 /* The maximum length command */

char hist[10][MAX_LINE];
char *outputRedirection;
char *inputRedirection;
int redirectOut, redirectIn;
int numOfCommands;

void parseRedirection(int redirection, char whichWay) {
    int task;
    int backup;
    if(redirection == 1 && whichWay == '0') {
        fflush(stdout);
        task = open(outputRedirection, O_CREAT | O_WRONLY, S_IRUSR);
        if(task != 0) {
            dup2(task, 1);
            close(task);
        }
    }
    else if(redirection == 1 && whichWay == 'I') {
        fflush(stdout);
        task = open(inputRedirection, O_RDONLY, S_IRUSR);
        if(task != 0) {
            dup2(task, 0);
            close(task);
        }
    }

}

void showHistory(){
    int i;
    int j = 0;
    int historyCount = numOfCommands;
    for (i = 0; i < 10; i++){
        printf("%d.  ", historyCount);
        while (hist[i][j] != '\n' && hist[i][j] != '\0'){
            printf("%c", hist[i][j]);
            j++;
        }
        printf("\n");
        j = 0;
        historyCount--;
        if (historyCount ==  0)
            printf("There is no history to show");
        break;
    }
}

void parseToken(char in[], char *args[],int *isAmp)
{
    int index = 0;    //where to place next char in args
    int commandLength; 		//length of command
    int nextCommand;  			//next command
    int backup;
    int redirection;
    char whichWay;
    //read user input
    commandLength = read(STDIN_FILENO, in, MAX_LINE);

    //return error if length < 0
    nextCommand = -1;
    if (commandLength == 0) {
        perror("No command was given");
        exit(0);
    }
    if (commandLength < 0){
        perror("Invalid command");
        exit(-1);
    }

    //check each character in in[]
    for (int i = 0; i < commandLength; i++) {
        switch (in[i]){
            case ' ':
            case '\t' :
                if(nextCommand != -1){
                    args[index] = &in[nextCommand];
                    index++;
                }
                in[i] = '\0';
                nextCommand = -1;
                break;

            case '\n':
                if (nextCommand != -1){
                    args[index] = &in[nextCommand];
                    index++;
                }
                in[i] = '\0';
                args[index] = NULL;
                break;

            case '&': 		// command followed with &, wait for child to terminate
                *isAmp = 1;
                in[i] = '\0';
                break;
            case '<': {
            redirectIn = 1;
            whichWay = 'I';
            inputRedirection = malloc(10 * sizeof(in[i]));
            strcpy(inputRedirection, &in[i]);
            parseRedirection(redirectIn, whichWay);

        }
            case '>': {
                whichWay = 'O';
                redirectOut = 1;
                outputRedirection = malloc(10 * sizeof(in[i]));
                strcpy(outputRedirection, &in[i]);
                parseRedirection(redirectOut, whichWay);
            }
            case'|':
            default :
                if (nextCommand == -1)
                    nextCommand = i;
        }
    }
    args[index] = NULL;

    //check to see if user inputed the history command and if there is a command history
    if(strcmp(args[0],"history") == 0){
        if(numOfCommands>0){
            showHistory();
        } else{
            printf("\nNo Commands in history\n");
        }
    }
        //Check to see if '!' is inputed
        //then retrieve appropriate command
    else if (**args == '!'){
        int check2 = args[0][1]- '0';
        int check3 = args[0][2];
        if(check2 > numOfCommands){
            printf("\nCommand not found\n");
            strcpy(in,"Invalid Command");
        }else if (check3 != 0){
            printf("\nInput too large.  History only contains last 10 commands. \n");
            strcpy(in,"Invalid Command");
        }	else{
            if(check2==-15){// If '!!' entered
                strcpy(in,hist[0]);

            }	else if(check2 == 0){
                printf("Can not enter 0. Enter ! followed by single integer 1-10");
                strcpy(in,"Invalid Command");
            }	else if(check2 >= 1){ //Checking for '!n', n >=1
                strcpy(in,hist[numOfCommands-check2]);
            }

        }
    }
    //update history

    for (int i = 10; i >=0; i--)
        strcpy(hist[i], hist[i-1]);
    strcpy(hist[0],in);
    numOfCommands++;
    if(numOfCommands>10){
        numOfCommands=10;
    }
}


int main(void)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    int in[MAX_LINE];
    pid_t pid;
    int amp;
    int counter;

    while (should_run) {
        printf("osh>");
        fflush(stdout);
        parseToken(in, args, &amp);
        pid = fork();

        if(pid < 0) {
            perror("Fork has failed. \n");
            exit(1);
        }
        else if (pid == 0) {
            if (execvp(args[0], args) == -1 && strcmp(in, "history") != 0) {
                perror("Invalid Command \n");
            }
        }
        else {
            counter++;
            if (amp == 0) {
                counter++;
                wait(NULL);
            }

        }
        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait() unless command included &
        */
    }
    return 0;
}
