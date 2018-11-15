#include <fstream>
#include <iostream>

using namespace std;

int main(){
   fstream file;
   const int SIZE = 1;
   char data; 
   file.open("index.html", ios::in | ios::binary);
   file.read(&data,1); 
   while(!file.eof()){
      cout << data; 
      file.read(&data,1);
      
   }
   //cout << endl; 
   
   return 0; 
}
