// AUTHOR: Joel Aguiar
// FILENAME: http_cli.cpp
// DATE: 11/15/18
// REVISION HISTORY: revised draft after first submission
// REFERENCES: n/a
//
// DESCRIPTION: This is a client http program that communicates with a web 
// server. It peforms a GET request to the server and it receives the
// returned header and, if available, body of the message. 

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
void requestResource(int, string, string);
int getHeader(int); 
void extractInfo(char*, string*, string*, string*);
void getPayload(int, int);

int const BUFFER_SIZE = 3000; 

// DESCRIPTION: the main method uses the command line arguments and other
// methods to communicate wit the server. 
// INPUT: argc- the number of command line arguments
//       argv- the command line arguments
// OUTPUT: returns int - exit status
int main(int argc, char** argv) {
   if(argc != 2) {
      cerr << "ERROR: incorrect number of command line arguments" << endl;
      exit(0); 
   }
   string portno;
   string hostname;
   string page; 
   extractInfo(argv[1], &portno, &hostname, &page); 
   //establish connection with server
   int sockfd = establishConnection(portno, hostname);
   //communicate with server
   communicate(sockfd, hostname, page);
   //close socket
   close(sockfd);
   return 0;
}

// DESCRIPTION: this method seperates the hostname, port number, and resource. 
// INPUT: s- original string with hostname, port number, and resource.
//       portno- a string pointer that will hold the port number. 
//       hostname- a string pointer that wil hold the hostname
//       page - a string pointer that will hold the resource location 
// OUTPUT: void. 
void extractInfo(char* s, string* portno, string* hostname, string* page){
   const char delim[2] = "/";
   char lastChar = s[strlen(s)-1];
   char *token;
   token = strtok(s, delim);
   int count = 0; //count of tokens
   string hnString = "";  
   string pgString = "/";
   string prString = "";  
   while( token != NULL) {
       if(count ==1){
            hnString += token; 
         } else if (count >= 2){
           pgString += token;
           pgString += "/"; 
         }
         
         token = strtok(NULL, delim);
         count++;
   }
   if(lastChar != '/'){
      pgString.pop_back();
   }   
   if (count < 3){
      pgString += "/";
   }

   //check for port number 
   string val = ":";
   size_t found = hnString.find(val,5);
   if(found == string::npos){
       prString = "80";
   }else{
      char hnArray[hnString.size()+1];
      strcpy(hnArray, hnString.c_str());
      const char delim2[2] = ":";
      char *token2;
      token2 = strtok(hnArray, delim2);
      hnString = token2;
      token2 = strtok(NULL, delim2);
      prString = token2;
   } 

   //save the content of the strings to the char pointers
   char hn[hnString.size()+1];
   strcpy(hn, hnString.c_str());
   *hostname = hn;
   
   char pg[pgString.size()+1];
   strcpy(pg, pgString.c_str());
   *page = pg;

   char pr[prString.size()+1];
   strcpy(pr, prString.c_str());
   *portno = pr;   
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

// DESCRIPTION: this method calls methods to sends the request to the web 
//       server program and to get the response. 
// INPUT: sockfd- socket
//       hostname - the web server hostname
//       page - the location of the resource
// OUTPUT: void
void communicate(int sockfd, string hostname, string page){
   requestResource(sockfd, hostname, page); 
   int response = getHeader(sockfd); 
   getPayload(sockfd, response);  
   cerr << endl << endl;
   
}


// DESCRIPTION: this method sends a GET request to the web server. 
// INPUT: sockfd - socket
//       hostname - the web server hostname
//       page -the location of the resource
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
      //send loop starts at buffer + n (number of bytes received)
      //assumes that request message is less than or equal to buffer size
      n += send(sockfd,(buffer+n), message.size(),0);
      if(n<0) {
         cerr << "ERROR: writing to socket" << endl;
      }
   }

}

// DESCRIPTION: this method gets the data from the response
// INPUT: newsockfd = socket used to communicate
//        size = size of the data that will be read. 
void getPayload(int newsockfd, int size){
   int n = 0; 
   char buffer[BUFFER_SIZE];
   bzero(buffer, BUFFER_SIZE);
   int index = 0; 
   while(n < size){
      n+= recv(newsockfd, (buffer+index), 1, 0);
      index += 1; 
      if(index == BUFFER_SIZE){
         for(int i=0; i< index; i++){
            cout << buffer[i];
         } 
         bzero(buffer, BUFFER_SIZE);
         index =0;     
      }
   }
   
   for(int i=0; i< index; i++){
      cout << buffer[i];
   } 
}

// DESCRIPTION: this method read and parses the header response from the web
// server. 
// INPUT: newsockfd - socket that is used to receive message
// OUPUT: int representing the size of the body if applicable
int getHeader(int newsockfd){
      char buffer[BUFFER_SIZE];
      bzero(buffer, BUFFER_SIZE);
      int n = 0; 
      int doneReadingHeader =0; 
      string header = ""; 
      do{
         n += recv(newsockfd,(buffer+n), 1, 0); 
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
      for(int i=0; i< n; i++){
         header += buffer[i];
      }       
      cerr << header; 
     
      char headerArr[n]; 
      strcpy(headerArr, header.c_str());
 
      //extract the number of bytes of body
      const char s[3] = "\r\n";
      char *token;
      token = strtok(headerArr, s); 
      char lengthFlag[16] = "Content-Length:";
      int lengthOfMessage;
      while( token != NULL) {
         string currentString(token); 
         char key[100];  
         sscanf(token, "%s", key);
         if(strcmp(key,lengthFlag) == 0){
            sscanf(token, "%s %d", key,&lengthOfMessage);
         }
         token = strtok(NULL, s);
      }
      
      return lengthOfMessage;
}
