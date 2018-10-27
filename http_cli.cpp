// AUTHOR: Joel Aguiar
// FILENAME: warmup_cli.cpp
// DATE: 10/16/18
// REVISION HISTORY: submitted draft
// REFERENCES: n/a
//
// DESCRIPTION: This is a client program that communicates with a server
// program. It sends the server some characters and then it receives the
// return message from the server which should be the same characters 
// reversed. 

#include "common.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h> 
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std; 

int establishConnection(string, string);
void communicate(int, string, string); 
string getMessage();
void requestResource(int, string, string);
int getHeader(int); 
string getSize(string); 
void extractInfo(string, string*, string*, string*);
void getPayload(int, int);


int const BUFFER_SIZE = 2000; 

// DESCRIPTION: the main method uses the command line arguments and other
// methods to communicate wit the server. 
// INPUT: argc- the number of command line arguments
//       argv- the command line arguments
// OUTPUT: returns int - exit status
int main(int argc, char** argv) {
   /*
   string message = getMessage(); 
   const int MAXIMUM_MESSAGE_SIZE = 127; 
   if(message.size() > MAXIMUM_MESSAGE_SIZE){
      cerr << "ERROR: your message must be equal"<< 
            " or less than 127 characters. Your"<<
            " message length is " << message.size() << ".\n"; 
      return 0;
   } 
   */ 
   string portno;
   string hostname;
   string page; 
   extractInfo(argv[1], &portno, &hostname, &page); 
   cerr <<  "DEBUG: portno: " << portno << " hostname: " << hostname << 
            " page: " << page << endl; 
   //establish connection with server
   int sockfd = establishConnection(portno, hostname);
   //communicate with server
   communicate(sockfd, hostname, page);
   if(sockfd == 0) {}; //FIXME
   return 0;
}

// DESCRIPTION: this method seperates the hostname and the port number. 
// INPUT: s- original string with hostname and port number.
//       portno- a string pointer that will hold the port number. 
//       hostname- a string pointer that wil hold the hostname
// OUTPUT: void. 
void extractInfo(string s, string* portno, string* hostname, string* page){
   *page = "/joelaguiar.com/"; //FIXME
   bool on = true; 
   //bool portFinished = false; 
   string apge = "";  
   string port = ""; 
   string host = "";
   for(unsigned int i=0; i<s.size(); i++){
      if(s.at(i) == ':'){
         on = false; 
      } else {
         if(on) {
            host += s.at(i); 
         } else {
            port += s.at(i); 
         }
      }   
   } 
   char h[host.size() +1];
   strcpy(h, host.c_str());

   char p[port.size() +1];
   strcpy(p, port.c_str());
   *portno = p; 
   *hostname = h;  
}

// DESCRIPTION: This method creates uses getaddrinfo to produce a socket to 
// establish a connection. 
// INPUT: portno- the port number
//       hostname- the host name
// OUTPUT: int that represents the socket
int establishConnection(string portno, string hostname){ 
   struct addrinfo hints; 
   struct addrinfo *result;
   memset(&hints, 0, sizeof(struct addrinfo)); 
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0; 
   hints.ai_protocol = 0; 


   char hname[hostname.size()+1]; 
   strcpy(hname, hostname.c_str());

   char pon[portno.size() +1];
   strcpy(pon, portno.c_str()); 

   int s = getaddrinfo(hname, pon, &hints, &result); 
   if(s != 0){
      cerr << "ERROR: with getaddrinfo" << endl;
      exit(0);
   }
   int sockfd = socket(result->ai_family, 
               result->ai_socktype, 
               result->ai_protocol);
   if(sockfd == -1){
      cerr << "ERROR opening socket" << endl; 
      exit(0);
   }

    if(connect(sockfd, result->ai_addr, result->ai_addrlen) <0){
      cerr << "ERROR connecting" << endl; 
      exit(0);
   }  
   
   return sockfd; 
}

// DESCRIPTION: this method sends the characters to the server program
// INPUT: sockfd- socket
//       message- the message that will be sent
// OUTPUT: void
void communicate(int sockfd, string hostname, string page){
   requestResource(sockfd, hostname, page); 
   int response = getHeader(sockfd); 
   cerr << response << endl; 
   getPayload(sockfd, response);  
   
}

// DESCRIPTION: this method gets the size of a string and returns the size as
// another string. 
// INPUT message - the message
// OUTPUT: the size in the form of a string
string getSize(string message){
   int size = message.size();
   const int MAX_EXPECTED_SIZE_OF_SIZE_OF_STRING = 4;  
   char buffer[MAX_EXPECTED_SIZE_OF_SIZE_OF_STRING]; 
   snprintf(buffer,sizeof(buffer), "%d", size);
   return buffer; 

}

// DESCRIPTION: this method sends a message to the server program. 
// INPUT: sockfd - socket
//       message - the characters that will be sent
// OUTPUT: void
void requestResource(int sockfd, string hostname, string page){
   //write message
   string message =  "GET " + page + " HTTP/1.1\r\n" +
                     "Host: " + hostname + "\r\n" +
                     "Connection: close\r\n\r\n"; 
   cerr << message << endl;   
   unsigned int n =0;  
   char buffer[BUFFER_SIZE]; 
   bzero(buffer,BUFFER_SIZE);
   strcpy(buffer, message.c_str());
   while(n < message.size()){
      n += write(sockfd,buffer, message.size());
      if(n<0) {
         cerr << "ERROR: writing to socket" << endl;
      }
   }

}

// DESCRIPTION: this method gets a message from the user through a prompt
// INPUT: none
// OUTPUT: the message the user typed
string getMessage(){
   string message;
   cout << "Enter a message: "; 
   getline(cin, message);
   return message; 
}  

void getPayload(int newsockfd, int size){
   int n = 0; 
   char buffer[BUFFER_SIZE];
   bzero(buffer, BUFFER_SIZE);
   string test; 
   cin >> test;    
   while(n < size){
      n+= read(newsockfd, (buffer+n), 1); 
      cerr << "n: " << n << endl; 
   }
   
   for(int i=0; i< n; i++){
      cout << buffer[i];
   } 
}

// DESCRIPTION: this method receives communication from the server. It reads 
// in the corect format to be able to interpret the message. 
// INPUT: newsockfd - socket that is used to receive message
// OUPUT: string that is the message received from the server. 
int getHeader(int newsockfd){
      cerr << "DEBUG: in getHeader" << endl; 
      char buffer[BUFFER_SIZE];
      bzero(buffer, BUFFER_SIZE);
      int n = 0; 
      int doneReadingHeader =0; 
      string header = ""; 
      do{
         n += read(newsockfd,(buffer+n), 1); 
         //FIXME if buffer overflow then do something
         if(buffer[n-1] == '\n' && doneReadingHeader == 0){
            doneReadingHeader = 1; 
         } else if (buffer[n-1] == '\r' && doneReadingHeader == 1){
            doneReadingHeader = 2;
         } else if (buffer[n-1] == '\n' && doneReadingHeader ==2){
            doneReadingHeader = 3; 
         } else {
            doneReadingHeader = 0; 
         }
      } while(doneReadingHeader <3);
      cerr << "N: " << n << endl; 
      for(int i=0; i< n; i++){
         header += buffer[i];
      }       
      cerr << header; 
     
      char str[n]; //FIXME bad name
      strcpy(str, header.c_str());
 
      //extract the number of bytes of body
      const char s[3] = "\r\n";
      char *token;
      token = strtok(str, s); 
      char lengthFlag[16] = "Content-Length:";
      int lengthOfMessage;
      while( token != NULL) {
         //printf( " %s\n", token);
         string currentString(token); 
         char key[100];  
         sscanf(token, "%s", key);
         if(strcmp(key,lengthFlag) == 0){
            sscanf(token, "%s %d", key,&lengthOfMessage);
            cerr << "FOUND IT: " << lengthOfMessage << endl;
         }
         token = strtok(NULL, s);
      }
      /*
      char buffer[BUFFER_SIZE];
      bzero (buffer, BUFFER_SIZE);
      int n =0; //number of bytes that represent the number of bytes of size
      int i =0; //number of bytes that represent the size of message
      int m =0; //number of bytes of message
      while(m < 1){
         m += read(newsockfd, (buffer), 1); 
         if(m < 0) {
            cerr << "ERROR: reading socket" << endl; 
            exit(0);
         }
      }
      const int ASCII_VALUE_FOR_ZERO = 48;
      int sizeOfSize = (int)buffer[0] - ASCII_VALUE_FOR_ZERO;
      bzero(buffer, BUFFER_SIZE);
      while(i < sizeOfSize){
         i += read(newsockfd, (buffer+i), 1);
         if(i < 0){
            cerr << "ERROR: reading socket" << endl; 
            exit(0);
         } 
      }

      string stringSize = "";
      for(int i= 0; i< sizeOfSize; i++){
         stringSize += buffer[i];
      }
      char buffer2[stringSize.size()];
      strcpy(buffer2, stringSize.c_str());
      int sizeOfMessage = atoi(buffer2);
      while(n < sizeOfMessage){
         n += read(newsockfd, (buffer+n), 1); 
      }
      string message = "";
      for(int i=0; i<sizeOfMessage; i++){
         message += buffer[i];
      }
      */
      return lengthOfMessage;
}
