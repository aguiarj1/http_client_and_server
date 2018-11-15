#!/bin/bash

clear
# write some test to start the server with bad command line args
# bad command line arguments
# check if file size matches and file content matches
# correct expect code 200
echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001/"
echo "EXPECTED OUTPUT: 200"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001/ > index.html


curl -v -s http://streetlevelhealthproject.org > curlIndex.html

#curl http://streetlevelhealthproject.org > curlIndex.html

diff -q index.html web_root/index.html

rm -f index.html
rm -f  curlIndex.html

echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001"
echo "EXPECTED OUTPUT: 200"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001 > index.hmtl
rm -f index.html
echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001/foo/"
echo "EXPECTED OUTPUT: 200"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001/foo/bar.png > index.hmtl
rm -f index.html

# case index is not there
echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001/foo/"
echo "EXPECTED OUTPUT: not found"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001/foo/ > index.html
rm -f index.html

# case index is not there
echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001/foo"
echo "EXPECTED OUTPUT: not found"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001/foo > index.html
rm -f index.html

# case index is not there
echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001/foo/bar"
echo "EXPECTED OUTPUT: not found"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001/foo/bar > index.html
rm -f index.html



# incorrect (have several for each code)

#test with file that is there but that isn't supported
echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001/icon.ico"
echo "EXPECTED OUTPUT: DOES NOT SUPPORT THAT TYPE"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001/icon.ico > icon.ico
rm -f icon.ico

echo "-------------------------------------------------"
echo "INPUT: ./http_cli http://10.124.72.20:10001/cat.jpg"
echo "EXPECTED OUTPUT: not found"
echo "-------------------------------------------------"
./http_cli http://10.124.72.20:10001/cat.jpg > cat.jpg
rm -f cat.jpg
# test for server errors
# test with a big file size.. big picture

# do some comparisons with curl and the same webpage/resource
