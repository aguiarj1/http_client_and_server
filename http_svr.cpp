// AUTHOR: Joel Aguiar
// FILENAME: http_svr.cpp
// DATE: 11/13/18
// REVISION HISTORY: submitted draft
// REFERENCES: n/a
//
// DESCRIPTION: This is a http server program that communicates with a http 
// client program. It waits for a client to submit a request it responds with
// and appropriate header and, when applicable, a body.  

#include "common.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream> 
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
//#include <bits/stdc++.h> //to reverse
#include <stdlib.h>   /* strtol */
#include <fstream>
#include <sys/stat.h>
#include <ctime>

using namespace std; 

int setUpSocketToListen(int);
void respondToHTTPrequests(int); 
void processBuffer(char*);
void sendMessage(int, string); 
void sendMessage(int, char*,int);
string receiveHeader(int); 
string getResourcePath(string); 
string generateHeader(string, string, string); 
string generateBody(int, string); 
string determineStatusCode(string, string &, string &); 
bool checkFileExtension(string, string &);

int const BUFFER_SIZE = 1000;
//FIXME do i have to worry about hton()?????


// DESCRIPTION: the main method uses a command line argument that represents
// the port number and other methods to communicate with the client. 
// INPUT: argc- the number of command line arguments
//       argv- the command line arguments
// OUTPUT: returns int - exit status
int main(int argc, char** argv) {
   if(argc != 2) {
      cerr << "ERROR: incorrect number of command line arguments" << endl;
      exit(0);
   } 
   int sockfd = setUpSocketToListen(atoi(argv[1]));
   respondToHTTPrequests(sockfd); 
   close(sockfd); 
   return 0;
}


string determineStatusCode(string message, string &revPath, string &contentType){
  //find where the first /r/n is and based on that get that substring
  size_t indexOfFirstLine = message.find("\r\n");
  if(indexOfFirstLine == string::npos){
      return "400";
  }
  string requestLine = message.substr(0,indexOfFirstLine);
  
   char requestLineCharArray[requestLine.size()];
   strcpy(requestLineCharArray, requestLine.c_str());
   char * token;
   token = strtok(requestLineCharArray, " "); 
   int countOfTokens =0; 
   while(token != NULL){
      if(countOfTokens ==0){ 
         //check if it a GET command
         if(strcmp(token, "GET") != 0){ 
            return "501"; // not implemented
         }
      } else if(countOfTokens ==1){  
         //check valid uri
         struct stat info;
         string path(token);
         if(path.find("/../") != string::npos){
            return "400";
         }
         char lastValue = path.at(path.length()-1);  
         revPath = "web_root" + path; 
         char cRevPath[revPath.size()];
         strcpy(cRevPath, revPath.c_str()); 
         if(stat(cRevPath, &info) != 0){ //verifies it can read the path
            return "404";
         } else {
            if((info.st_mode & S_IFMT) == S_IFREG) {
               //check to see if it is a file
               //if correct file extention
               if(checkFileExtension(path, contentType)){
                  return "200";
               } else {
                  return "501"; //not implemented for that file type
               }
            } else { //it is not a regular file
               if(lastValue == '/'){
                  revPath = revPath + "index.html";
               } else {
                  revPath = revPath + "/index.html";
               }
               struct stat info2;
               char secondRevPath[revPath.size()];
               strcpy(secondRevPath, revPath.c_str()); 
               if(stat(secondRevPath, &info2) != 0){ //verifies it can read the path
                  return "404";
               } else {
                  contentType = "text/html";
                  return "200";
               }
            }  
         }
      }
      countOfTokens++; 
      token = strtok(NULL, " "); 
   } 
   return "400";
}
// DESCRIPTION: this method listens and sends messages to client program
// INPUT: sockfd- socket 
// OUPUT:void
void respondToHTTPrequests(int sockfd){
   while(true){
      struct sockaddr_in cli_addr;
      socklen_t clilen = sizeof(cli_addr);
      //accept connection
      int newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
      if(newsockfd < 0){
         cerr << "ERROR: on accept" << endl; 
         exit(0);
      }
      string requestHeader = receiveHeader(newsockfd); 
      cerr << requestHeader << endl; 
      string path;
      string contentType;
      string statusCode = determineStatusCode(requestHeader,path,contentType); 
      string header = generateHeader(path, statusCode,contentType);   
      sendMessage(newsockfd, header);  
      if(strcmp(statusCode.c_str(), "200") == 0){
         string body = generateBody(newsockfd, path);  
      }
      close(newsockfd);
   } 
}

bool checkFileExtension(string path, string &contentType){
      int period = path.find(".");
      //FIXME assumes it is positive
      string fileExtension = path.substr(period+1); 
      if(fileExtension == "txt"){
            contentType = "text/plain";
            return true; 
      } else if(fileExtension == "html"){
            contentType = "text/html";
            return true; 
      } else if(fileExtension == "htm"){
            contentType = "text/htm";
            return true; 
      } else if(fileExtension == "css"){
            contentType = "text/css";
            return true; 
      } else if(fileExtension == "jpg" || fileExtension == "jpeg"){
            contentType = "image/jpeg";
            return true; 
      } else if(fileExtension == "png"){
            contentType = "image/png";
            return true; 
      } else {
         return false; 
      }
}

string generateHeader(string path, string statusCode, string contentType){
   string response = "";
   response += "HTTP/1.1 ";
   if(strcmp(statusCode.c_str(), "200") == 0){           
      time_t now = time(0);
      tm *ltm = localtime(&now);
      char buffer[200];
      strftime(buffer,200,"%a, %d %h %G %T %Z",ltm);
      string currTime(buffer);
      

      struct stat sb; 
      char cPath[path.size()];
      strcpy(cPath, path.c_str());
      if(stat(cPath, &sb) == -1) {
         cerr << "ERROR gettings stats from file." << endl;
         exit(EXIT_FAILURE);
      }
      long length = (long)sb.st_size;
      struct timespec lastModified = sb.st_mtim;
      time_t lastMod = lastModified.tv_sec; 
      tm *ltm2 = localtime(&lastMod);
      char buffer2[200];
      strftime(buffer2,200,"%a, %d %h %G %T %Z",ltm2);
      string lastModString(buffer2);
     

      response +="200 OK\r\n";
      response += "Content-Length: ";
      response += to_string(length);
      response += "\r\n"; //FIXME
      response += "Date: ";
      response += currTime;
      response += "\r\n";
      response += "Last-Modified: ";
      response += lastModString;
      response += "\r\n";
      response += "Content-Type: "; 
      response += contentType;
      response += "\r\n";
   } else if(strcmp(statusCode.c_str(), "501") == 0){
      response += "501 Not Implemented\r\n"; 
      response += "Content-Length: 0\r\n";
   } else if(strcmp(statusCode.c_str(), "404") == 0){
      response += "404 Not Found\r\n"; 
      response += "Content-Length: 0\r\n";
    } else if(strcmp(statusCode.c_str(), "400") == 0){
      response += "400 Bad Request\r\n"; 
      response += "Content-Length: 0\r\n";
  
  } 
   response += "Connection: close\r\n";
   response += "\r\n";
   
   return response; 
}

string generateBody(int sockfd, string path){
   //FIXME when they don't provide file name I have to provide
   //FIXME watch out for backslaces... do I need two?
   //get length of file:
   //send the file in parts that fit in the buffer 
   //FIXME assumes it is greater 
   //FIXME body is not necessarily a string 
   fstream file;
   //const int SIZE = 1;
   char data;
   file.open(path, ios::in | ios::binary);
   if(file.is_open()){
      file.read(&data,1);
      int n =0;
      while(!file.eof()){
         n += write(sockfd,&data, 1); 
         if(n<0) {
            cerr << "ERROR: writing to socket" << endl;
            exit(0);
         }
         file.read(&data,1);

      }
   } else {
      cerr << "couln't open file" << endl; 

   }
   cerr << endl;
   return "TEST BODY";

}


// DESCRIPTION: this method reverses the order of a string
// INPUT: message- the string that will be reversed. 
// OUTPUT: the reversed string. 
string getResourcePath(string message){
   char cMessage[message.size()+1];
   strcpy(cMessage, message.c_str());
   string path = "";  
   //parse message for what I need to return
   int result = message.find("GET", 0);
   if(result >= 0){  // GET request
      //get second group of letters seperate by spaces: 
      const char delim[2]= " ";
      char *token; 
      token = strtok(cMessage, delim); 
      token = strtok(NULL, delim); 
      path += token;  
   } else { //not GET request
      
   } 
   string rootFolder = "web_root";
   string completePath = rootFolder + path; 
   if(completePath.substr(completePath.size()-1,1) == "/"){
      completePath += "index.html";
   } 
   return completePath; 
}


// DESCRIPTION: this method takes care of listening for incoming communication
// and parsing the message. 
// INPUT: newsockfd - the socket.
// OUTPUT: string - the string that represents what the client sent to server
string receiveHeader(int newsockfd){
      char buffer[BUFFER_SIZE];
      bzero (buffer, BUFFER_SIZE);
      int n =0; 
      int doneReadingHeader = 0;
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
      return header;  
}

// DESCRIPTION: this method initiates the socket to be able to communicate
// INPUT: portno- port number
// OUTPUT: int that represent the socket created. 
int setUpSocketToListen(int portno){
   int sockfd;
   struct sockaddr_in serv_addr;
   //create a TCP socket
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if(sockfd < 0){
      cerr << "ERROR: making socket" << endl;
      exit(0); 
   }

   //Assign a port number 
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;  
   serv_addr.sin_port = htons(portno);
   int resultOfbind = bind(sockfd, 
                          (struct sockaddr *) &serv_addr, 
                           sizeof(serv_addr)); 
   if(resultOfbind != 0) {
      cerr << "ERROR: bind" << endl; 
      exit(0);
   }
   //set to listening 
   listen(sockfd,5); 
   return sockfd; 
}

// DESCRIPTION: this mehtod sends a message to the client. 
// INPUT: sockfd - socket used to communicate
//       message - message that will be sent to client
// OUTPUT: void 
void sendMessage(int sockfd, string message){
   unsigned int n =0;
   char buffer[BUFFER_SIZE]; 
   bzero(buffer,BUFFER_SIZE);
   strcpy(buffer, message.c_str());
   while(n < message.size()){ 
      n += write(sockfd,(buffer+n), 1); 
      if(n<0) {
         cerr << "ERROR: writing to socket" << endl;
         exit(0);
      }
   }

}


// DESCRIPTION: this mehtod sends a message to the client. 
// INPUT: sockfd - socket used to communicate
//       message - message that will be sent to client
// OUTPUT: void 
void sendMessage(int sockfd, char* buffer, int size){
   int n =0;
   while(n < size){ 
      n += write(sockfd,(buffer+n), 1); 
      if(n<0) {
         cerr << "ERROR: writing to socket" << endl;
         exit(0);
      }
   }
}
