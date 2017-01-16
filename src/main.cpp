#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>

#define MSG_SIZE 256

using namespace std;

//pipes for output and translate
int outPipeFd[2], transPipeFd[2], transOutPipeFd[2];

//current pid
pid_t pid;

//buffer for reading
char inbuff[MSG_SIZE];



void output(){
    printf("output proc\n");
    while(1){
        read(outPipeFd[0],inbuff,MSG_SIZE);
        printf("%s\n",inbuff);
        read(transOutPipeFd[0],inbuff,MSG_SIZE);
        printf("%s\n",inbuff);
    }
}

void input(){
    printf("input proc\n");
    string s;
    while(getline(cin,s)){
        write(outPipeFd[1],s.c_str(),s.size()+1);
        write(transPipeFd[1],s.c_str(),s.size()+1);
    }
    kill(pid,SIGTERM);
    wait(static_cast<int*>(0));
}

void translate(){
    printf("translate proc\n");
    while(1){
        read(transPipeFd[0],inbuff,MSG_SIZE);
        string s(inbuff,strlen(inbuff));
        replace(s.begin(),s.end(),'a','z');
        write(transOutPipeFd[1],s.c_str(),s.size()+1);
    }
    kill(pid,SIGTERM);
    wait(static_cast<int*>(0));
}
int main(int argc, char *argv[]){
    if(pipe(outPipeFd) < 0 || pipe(transPipeFd) < 0 || pipe(transOutPipeFd)){
        perror("Failed to open pipes\n");
        exit(1);
    }
    pid = fork();
    switch(pid){
        case 0://child
            pid = fork();
            switch(pid){
                case 0://child - output
                    output();
                    break;
                case -1:
                    perror("failed second fork\n");
                    exit(3);
                    break;
                default://parent - translate
                    translate();
                    break;
            }
            break;
        case -1:
            perror("failed first fork\n");
            exit(2);
            break;
        default://parent - input
            input();
            break;
    }
}


