#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#define MSG_SIZE 256

using namespace std;

//pipes for output and translate
int outPipeFd[2], transPipeFd[2], transOutPipeFd[2];

//child pid
pid_t pid;
//flag for killing loop from sig handler
bool running = 1;

//functions for the processes
void output();
void input();
void translate();

//shared signal handler
void signal_handler(int sig);

int main(int argc, char *argv[]){
    //open pipes
    if(pipe(outPipeFd) < 0 || pipe(transPipeFd) < 0 || pipe(transOutPipeFd)){
        perror("Failed to open pipes\n");
        exit(1);
    }
    //switch to non blocking mode
    if(fcntl(outPipeFd[0], F_SETFL, O_NDELAY) < 0  || fcntl(transPipeFd[0], F_SETFL, O_NDELAY) < 0 || fcntl(transOutPipeFd[0], F_SETFL, O_NDELAY) < 0){
        perror("fnctrl error");
        exit(1);
    }

    pid = fork();
    switch(pid){
        case -1:
            perror("failed first fork\n");
            exit(2);
            break;
        case 0://child
            pid = fork();
            switch(pid){
                case -1:
                    perror("failed second fork\n");
                    exit(3);
                    break;
                case 0://child - output
                    output();
                    break;
                default://parent - translate
                    translate();
                    break;
            }
            break;
        default://parent - input
            input();
            break;
    }
    return 0;
}

void signal_handler(int sig){
    running = 0;
}

void output(){
    //buffer for reading
    char inbuff[MSG_SIZE];
    while(running){
        if(read(outPipeFd[0],inbuff,MSG_SIZE)>0)
            printf("%s\r\n",inbuff);
        if(read(transOutPipeFd[0],inbuff,MSG_SIZE)>0)
            printf("%s\r\n",inbuff);
    }
}

void input(){
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGTERM, &sa, NULL);

    system("stty raw igncr -echo");
    
    vector<char> buffer;
    char c;
    string s;
    
    while(running){
        switch((c = getchar())){
            case 'X':
                buffer.pop_back();
                break;
            case 'K':
                buffer.clear();
                break;
            case 'T':
                running = false;
                //fall through and complete last line
            case 'E':
                s = string(buffer.begin(),buffer.end());
                write(outPipeFd[1],s.c_str(),s.size()+1);
                write(transPipeFd[1],s.c_str(),s.size()+1);
                buffer.clear();
                break;
            case 11:
                exit(0);
                break;
            default:
                buffer.push_back(c);
                break;
        }
    }
    kill(pid,SIGTERM);
    wait(static_cast<int*>(0));
    system("stty -raw -igncr echo");
}

void translate(){
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGTERM, &sa, NULL);
    
    //buffer for reading
    char inbuff[MSG_SIZE];
    
    while(running){
        if(read(transPipeFd[0],inbuff,MSG_SIZE)>0) {
            string s(inbuff,strlen(inbuff));
            replace(s.begin(),s.end(),'a','z');
            write(transOutPipeFd[1],s.c_str(),s.size()+1);
        }
    }

    kill(pid,SIGTERM);
    wait(static_cast<int*>(0));
}

