//SEP400 - Assignment 1: Embedded Debug Logging
//LogServer.cpp - Server file to commnunicate with the logger file (Client to Buyer)
//Created: Abhi Patel -> apatel477@myseneca.ca
//         Neel Mahimkar -> nmahimkar@myseneca.ca
//Date: Mar 26, 2023.

#include <arpa/inet.h>
#include <iostream>
#include <queue>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include<fstream>
#include<limits>
#include<stdio.h>
#include<stdlib.h>


using namespace std;

const int PORT=1154;
const char IP_ADDR[]="127.0.0.1";
const int BUF_LEN=4096;
bool is_running=true; //flag
struct sockaddr_in remaddr; //socket
void *recv_thread(void *arg); //receive thread function
pthread_mutex_t lock_x; //mutex

//The server will shutdown gracefully via a ctrl-C via a shutdownHandler.
void shutdownHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
            break;
    }
}

void *recv_thread(void *arg) {//servers receiver thread
    int fd=*(int *)arg;
    socklen_t addrlen=sizeof(remaddr);
    char buf[BUF_LEN];
    memset(buf,0,BUF_LEN);
    cout<<"recv_thread: STARTING...\n"<<endl;//MW: useful debug [WORKING]

    FILE* logFile=fopen("server_log.txt","a+"); //take any content from recvfrom() and write to the server log file.

    while(is_running){//run in an endless loop
	int len = recvfrom(fd, buf, BUF_LEN, MSG_DONTWAIT, (struct sockaddr *)&remaddr, &addrlen);
	
	//check for received data and write to the log file
	if (len > 0) {
        fputs(buf,logFile);
        }
	else{
        sleep(1);
        }
    }
    fclose(logFile);
    pthread_exit(NULL);
}

int main(void) {
    struct sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); //The server's main() function will create a non-blocking socket for UDP communications (AF_INET, SOCK_DGRAM).
    int ret;
    char buf[BUF_LEN];

    signal(SIGINT,shutdownHandler); //signal handler

    sockfd=socket(AF_INET, SOCK_DGRAM |SOCK_NONBLOCK, 0);
    if(sockfd<0) {
        cout<<"Cannot create the socket"<<endl;
        cout<<strerror(errno)<<endl;
        return -1;
    }


    memset(&servaddr, 0, sizeof(servaddr));  //Set socket addr into position zero 
    servaddr.sin_family = AF_INET;  
    servaddr.sin_port = htons(PORT);
    ret = inet_pton(AF_INET, IP_ADDR, &servaddr.sin_addr);
    if(ret==0) {
        cout<<"No such address"<<endl;
        cout<<strerror(errno)<<endl;
        close(sockfd);
        return -1;
    }

    //The server's main() function will bind the socket to its IP address and to an available port.
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        cout << "Failed to bind socket" << endl;
        cout<<strerror(errno)<<endl;
        return -1;
    }

    //The server's main() function will create a thread function and apply mutex to any shared resources.
    pthread_mutex_init(&lock_x, NULL);
    pthread_t tid;
    ret = pthread_create(&tid, NULL, recv_thread, &sockfd);
    if(ret!=0) {
        cout<<"Cannot start thread"<<endl;
        cout<<strerror(errno)<<endl;
        close(sockfd);
        return -1;
    }

    int option;
    int len;

    while(is_running){
	int option;
        cout << " 1. Set the log level" << endl;
        cout << " 2. Dump the log file here" << endl;
        cout << " 0. Shut Down" << endl;
        cin >> option;

	//Handles Invalid output if a string is entered
	if(cin.fail()){
	    cout<<"Error: Enter an int val"<<endl;
	    cin.clear();
	    cin.ignore(numeric_limits<streamsize>::max(),'\n');
	}
	//Handles Shut Down for the server
	else if(option==0){
        is_running=false;
        }
	// Set up the Log Level for the logger and sends the message to the logger
	else if(option==1){ 
		int level;
        cout<<"What Level? [0]-DEBUG, [1]-WARNING, [2]-ERROR, [3]-CRITICAL: ";
        cin>>level;
		memset(buf, 0, BUF_LEN);
		len=sprintf(buf, "Set Log Level=%d", level)+1;
        sendto(sockfd, buf, len, 0, (struct sockaddr *)&remaddr, sizeof(remaddr));
	}
	//Dumps the file onto the screen
	else if(option==2){
		FILE* logFile;
		char buf[BUF_LEN];

		//opens the logfile.txt in read mode
		logFile=fopen("./server_log.txt","r"); //The server will open its server log file for read only.
		if(logFile==NULL){
		    cout<<"Error: Unable to open the log file"<<endl;
		}
		//print out the data from the logFile
		while(fgets(buf,BUF_LEN,logFile)){
		    printf("%s",buf);
		}
		//close the log file and wait for the user to enter a key
		fclose(logFile);
		cout<<endl;
		cin.ignore();
		cout<<"Press any key to continue"<<endl;
		cin.get();
	}
	//Handles Invalid Output for integer
	else{
		cout<<"Invalid Selection"<<endl;
	}
    }
    
    cout<<"[DEBUG] Quitting Server..."<<endl;
    pthread_join(tid, NULL);
    close(sockfd);
    return 0;
}
//**Assignment 1**