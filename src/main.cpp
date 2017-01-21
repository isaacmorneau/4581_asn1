#include "main.h"

using namespace std;

//pipe from input to output
int outPipeFd[2];
//pipe from input to translate
int transPipeFd[2];
//pipe from translate to output
int transOutPipeFd[2];
/**
 * Function: main
 * 
 * Date: 2017/01/15
 *
 * Designer: Isaac Morneau; A00958405
 *
 * Programmer: Isaac Morneau; A00958405
 *
 * Interface: int main(void)
 *
 * Return: int - the return value for the whole application
 *
 * Notes: Main opens the pipes for communication between input, translate, and output.
 * It sets the terminal to raw mode then it forks twice to create the afore mentioned processes. 
 */
int main(void){
    //open pipes
    if(pipe(outPipeFd) < 0 
            || pipe(transPipeFd) < 0 
            || pipe(transOutPipeFd) < 0){
        perror("Failed to open pipes\n");
        exit(1);
    }
    //switch to non blocking mode
    if(fcntl(outPipeFd[0], F_SETFL, O_NDELAY) < 0  
            || fcntl(transPipeFd[0], F_SETFL, O_NDELAY) < 0 
            || fcntl(transOutPipeFd[0], F_SETFL, O_NDELAY) < 0){
        perror("fnctrl error\n");
        exit(1);
    }
    //set the terminal to raw mode
    system("stty raw igncr -echo");

    switch(fork()){
        case -1:
            perror("failed first fork\n");
            exit(2);
            break;
        case 0://child
            switch(fork()){
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

void output(void){
    //buffer for reading
    char inbuff[MSG_SIZE];
    char c;
    int readO,readT;

    //close unused handles
    close(outPipeFd[1]);
    close(transPipeFd[0]);
    close(transPipeFd[1]);
    close(transOutPipeFd[1]);

    while(1){
        if((readO = read(outPipeFd[0],&c,1))>0)
            printf(">%c",c);
        if((readT = read(transOutPipeFd[0],inbuff,MSG_SIZE))>0)
            printf("!%s\r\n",inbuff);
    }
}

void input(void){
    vector<char> buffer;
    char c, *outbuff;
    string s;
    bool running = 1;
    close(outPipeFd[0]);
    close(transPipeFd[0]);
    close(transOutPipeFd[0]);
    close(transOutPipeFd[1]);
    
    while(running){
        c = getchar();
        //always send to output
        write(outPipeFd[1],&c,1);
        switch(c){
            case EOF:
            case 'T'://terminate
                running = 0;
                //fall through and complete last line
            case 'E'://enter line
                outbuff = static_cast<char*>(malloc(buffer.size()*sizeof(char)));
                copy(buffer.begin(),buffer.end(),outbuff);
                write(transPipeFd[1],outbuff,buffer.size());
                buffer.clear();
                break;
            case 11://ctrl-k
                exit(0);
                break;
            default://non control character
                buffer.push_back(c);
                break;
        }
    }
    //reset terminal to normal mode before exit
    system("stty -raw -igncr echo");
    //kill all processes in process tree
    kill(0,SIGTERM);
    wait(static_cast<int*>(0));
}

void translate(void){
    //buffer for reading
    char inbuff[MSG_SIZE];
    int nread;

    //close unused pipes
    close(outPipeFd[0]);
    close(outPipeFd[1]);
    close(transPipeFd[1]);
    close(transOutPipeFd[0]);

    while(1){
        if((nread = read(transPipeFd[0],inbuff,MSG_SIZE))>0) {
            for(int i = 0; i < nread; i++) {
                if(inbuff[i] == 'X'){
                    memmove(inbuff+i-1,inbuff+i+1,nread - i);
                    nread -= 2;
                    if(i>=2)
                        i -= 2;
                } else if(inbuff[i] == 'K') {
                    memmove(inbuff,inbuff+i+1,nread - i);
                    nread -= i;
                    i = 0;
                } else if(inbuff[i] == 'a') {
                    inbuff[i] = 'z';
                }
            }
            write(transOutPipeFd[1],inbuff,strlen(inbuff)+1);
        }
    }
}

