// AUTHOR: Joel Aguiar
// FILENAME: http_svr.cpp
// DATE: 11/14/18
// REVISION HISTORY: submitted version
// REFERENCES: n/a
//
// DESCRIPTION: This is a http server program that communicates with a http 
// client program. It waits for a client to submit a request and it responds
// with and appropriate header and, when applicable, a body.  

#include "common.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream> 
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>   /* strtol */
#include <fstream>
#include <sys/stat.h>
#include <ctime>
#include <vector>

using namespace std; 

int setUpSocketToListen(int);
void respondToHTTPrequests(int); 
void processBuffer(char*);
void sendMessage(int, string); 
string receiveHeader(int); 
string generateHeader(string, string, string); 
void sendResource(int, string); 
string determineStatusCode(vector <string>, string &, string &); 
bool checkFileExtension(string, string &);
vector <string> parseRequestHeader(string);


int const BUFFER_SIZE = 3000;

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

// DESCRIPTION: This method reads in a request message and parses the first
// line. It puts the results in a vector.
// INPUT: message - contains the whole request message
// OUTPUT: vector<string> - containing the values in the frist line
vector <string> parseRequestHeader(string message){
   vector<string> result;
   size_t indexOfFirstLine = message.find("\r\n");
   if(indexOfFirstLine == string::npos){
      return result;
   }
   string requestLine = message.substr(0,indexOfFirstLine);
   
   char requestLineCharArray[requestLine.size()];
   strcpy(requestLineCharArray, requestLine.c_str());
   char * token;
   token = strtok(requestLineCharArray, " "); 
   while(token != NULL){
      result.push_back(token); 
      token = strtok(NULL, " "); 
   }
   return result; 
}


// DESCRIPTION: This method parses the request line and determines what is the 
// appropriate http reponse code. 
// INPUT: message - contains the first line of the request message in a vector
//       path - a string that represents the path of the resource
//       contentType - the content type is parsed from the path
// OUTPUT: a string that represents the numberical response code
string determineStatusCode(vector <string> message, string &path, 
                           string &contentType){
   //check to make sure it has the correct amount of values
   if(message.size() != 3){
      return "400";
   }
  //check if it a GET command
  if(message.at(0) !=  "GET"){ 
      // not implemented
      return "501";
  }
  //check to see if it has invalid /../
  if(message.at(1).find("/../") != string::npos){
      return "400";
  }
  //check proper HTTP version
  if(message.at(2) != "HTTP/1.1"){
      return "501"; 
  }

   path = message.at(1);
   //check if the path is referencing a folder path
   if(path.find(".") == string::npos){
      char lastValue = path.at(path.length()-1);
      //if folder, add the index.html to the path
      if(lastValue == '/'){
         path += "index.html";
      } else { 
         path += "/index.html";
      } 
   }
   path = "web_root" + path; 
   struct stat info;
   //check to see if it is a valid file
   char cRevPath[path.size()];
   strcpy(cRevPath, path.c_str()); 
   if(stat(cRevPath, &info) != 0){ //verifies it can read the path
      return "404";
   } else {
      if((info.st_mode & S_IFMT) == S_IFREG) {
         //check to see if it is a file ext supported
         if(checkFileExtension(path, contentType)){
            return "200";
         } else {
            return "501"; //not implemented for that file type
         }
      } else { //it is not a regular file, so no support
         return "501";     
            
      }  
   } 
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
      vector <string> requestLineElements;
      requestLineElements = parseRequestHeader(requestHeader);
      string statusCode = determineStatusCode(requestLineElements,path,contentType); 
      string header = generateHeader(path, statusCode,contentType);   
      sendMessage(newsockfd, header);  
      if(strcmp(statusCode.c_str(), "200") == 0){
         sendResource(newsockfd, path);  
      }
      close(newsockfd);
   } 
}

// DESCRIPTION: This method checks whether the file extension referenced in
// the path has been implemented in the program. If so, returns true, 
// otherwise it returns false.
// INPUT: path- the full path of the resource
//       contentTpe - the content type being requested
// OUTPUT: true if filetype implemented or false if not.
bool checkFileExtension(string path, string &contentType){
      size_t period = path.find(".");
      if(period == string::npos){
         return false; 
      }
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

// DESCRIPTION: This method makes an appropriate response header based on the 
// status code.  
// INPUT: path - the path of the resource
//       statusCode - the http status code
//       contentType - the filetype of the resource
// OUTPUT: a string representing the response header.  
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
      response += "200 OK\r\nContent-Length: " + to_string(length) + "\r\n" + 
                  "Date: " + currTime + "\r\nLast-Modified: " + lastModString +
                  "\r\nContent-Type: " + contentType + "\r\n";
   } else if(strcmp(statusCode.c_str(), "501") == 0){
      response += "501 Not Implemented\r\nContent-Length: 0\r\n";
   } else if(strcmp(statusCode.c_str(), "404") == 0){
      response += "404 Not Found\r\nContent-Length: 0\r\n";
    } else if(strcmp(statusCode.c_str(), "400") == 0){
      response += "400 Bad Request\r\nContent-Length: 0\r\n"; 
  } 
   response += "Connection: close\r\n\r\n";
   
   return response; 
}

// DESCRIPTION: This method sends the content of the resource seeked to the 
// client.
// INPUT: sockfd- socket used to communicate with client
//       path - path of the resource
void sendResource(int sockfd, string path){
   fstream file;
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
      cerr << "ERROR: couln't open file" << endl;
      exit(0);
   }

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
