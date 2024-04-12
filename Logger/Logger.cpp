//SEP400 - Assignment 1: Embedded Debug Logging
//Logger.cpp - Logger file to commnunicate with the server and client using UDP connection
//Created: Abhi Patel -> apatel477@myseneca.ca
//         Neel Mahimkar -> nmahimkar@myseneca.ca
//Date: Mar 26, 2023.
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <queue>
#include <arpa/inet.h>
#include <unistd.h>
#include "Logger.h"

using namespace std;

const int BUF_LEN=4096;
char buf[BUF_LEN];
bool is_running; //flag = true
const char IP_ADDR[]="127.0.0.1";
const int PORT=1154;//MW: Your server's address is 127.0.0.1:1154
struct sockaddr_in serveraddr; // socket
struct sockaddr_in loggeraddr;
pthread_mutex_t lock_x; //mutex log
void *recv_thread(void *arg); //receive thread function
pthread_t tid;
queue<string> Message;
LOG_LEVEL logLevel;

int sockfd;
int ret; 
int n;

int InitializeLog(){//Function called to initialize the logger

  socklen_t addrlen = sizeof(serveraddr);
  //TODO:     // Set the address and port of the server
  sockfd=socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);// Create a non-blocking socket for UDP communications
  if(sockfd<0) {
    cout<<"Socket cannot be created"<<endl;
    cout<<strerror(errno)<<endl;
    return -1;
    }
    
    //***create client socket address***
    memset(&serveraddr, 0, sizeof(serveraddr)); //This line clears the serveraddr structure by setting all its bytes to zero.
    loggeraddr.sin_family = AF_INET;
    loggeraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    loggeraddr.sin_port= htons(8080);//port number will be different here

    //Bind the client to Log address
    ret = bind(sockfd, (struct sockaddr *)&loggeraddr, sizeof(loggeraddr));
    if(ret<0) {
        cout<<"Cannot bind the socket to the local address"<<endl;
        cout<<strerror(errno)<<endl;
        return -1;
    }

    //**Set up the socket address structure for the server**
    memset((char*)&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    ret = inet_pton(AF_INET, IP_ADDR, &serveraddr.sin_addr);//set the IP address to INADDR_ANY and bind the socket sockfd to the serveraddr
    serveraddr.sin_port = htons(PORT);
    if(ret==0) {
      cout<<"Invalid Address"<<endl;
      cout<<strerror(errno)<<endl;
      close(sockfd);
      return -1;
      }

    cout<<"CONNECTED To Server on:"<<endl;
    cout<<"IP Address: "<<inet_ntoa(serveraddr.sin_addr)<<endl;
    cout<<"PORT: "<<ntohs(serveraddr.sin_port)<<endl;

      pthread_mutex_init(&lock_x, NULL); 
      is_running = true;

      ret = pthread_create(&tid, NULL, recv_thread, &sockfd);
      if(ret!=0) {
        cout<<"Cannot start thread!"<<endl;
        cout<<strerror(errno)<<endl;
        close(sockfd);
        return -1;
    }

    return 0; // must return an int
}


void SetLogLevel(LOG_LEVEL level){//function log level
    cout << "Setting Log Level" << endl;
    logLevel = level;    // Set the log level
    cout<<"logLevel: "<<logLevel<<endl;
}


void Log(LOG_LEVEL level, const char *file, const char *func, int line, const char *message){
  
  int send_to;
  char buf[BUF_LEN];
  // Compare the severity of the log to the filter log severity level
  if(level < logLevel){
    return;
   }
   
    // Create a timestamp to be added to the log message
   time_t now = time(0);
   char *dt = ctime(&now);
   memset(buf, 0, BUF_LEN);
   char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
   n = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], file, func, line, message)+1;
   buf[n-1]='\0';
   
   // Send the message to the server via UDP  
   send_to = sendto(sockfd,buf , n, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
}


void ExitLog(){   // Stop the receive thread via an is_running flag
    cout << "ExitLog()"<< endl;
    is_running = false;
    pthread_join(tid, NULL);
    close(sockfd);
}


void *recv_thread(void *arg){
    int sockfd = *(int *)arg;//declares an integer variable sockfd and initializes it to the integer value pointed to by the argument arg.

   // struct sockaddr_in serveraddr;
    socklen_t addrlen = sizeof(serveraddr);
    char buf[BUF_LEN];
    cout << "server: reading -> Levels ready to be dumped" << endl; //useful debug
    
    while(is_running) {
      memset(buf, 0, BUF_LEN);
      int n = recvfrom(sockfd, buf, BUF_LEN, MSG_DONTWAIT, (struct sockaddr *)&serveraddr, &addrlen);

      if(n < 0) {
        sleep(1);
        }
	//Else Print out the received message and set the Severity level of the message according to the message
       	else {
            buf[n] = '\0';
            if (strcmp(buf, "Set Log Level=0") == 0) {
                cout << "Setting Severity Level To  DEBUG" << endl;
                logLevel = DEBUG;
            } else if (strcmp(buf, "Set Log Level=1") == 0) {
                cout << "Setting Severity Level To  WARNING" << endl;
                logLevel = WARNING;
            } else if (strcmp(buf, "Set Log Level=2") == 0) {
              cout<<"Setting Severity Level To  ERROR"<<endl;
                logLevel = ERROR;
            } else if (strcmp(buf, "Set Log Level=3") == 0) {
              cout<<"Setting Severity Level To  CRITICAL"<<endl;
                logLevel = CRITICAL;
            }
        }
  }

  cout<<"[DEBUG] Quitting thread...success!"<<endl;
  pthread_exit(NULL);

}
