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
    while(1){
        if(read(outPipeFd[0],inbuff,MSG_SIZE)>0)
            printf("%s\r\n",inbuff);
        if(read(transOutPipeFd[0],inbuff,MSG_SIZE)>0)
            printf("%s\r\n",inbuff);
    }
}

void input(void){
    vector<char> buffer;
    char c;
    bool running = 1;
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
                running = 0;
                //fall through and complete last line
            case 'E':
                s = string(buffer.begin(),buffer.end());
                write(outPipeFd[1],s.c_str(),s.size()+1);
                write(transPipeFd[1],s.c_str(),s.size()+1);
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
    char inbuff[MSG_SIZE];
    while(1){
        if(read(transPipeFd[0],inbuff,MSG_SIZE)>0) {
            string s(inbuff,strlen(inbuff));
            replace(s.begin(),s.end(),'a','z');
            write(transOutPipeFd[1],s.c_str(),s.size()+1);
        }
    }
}

