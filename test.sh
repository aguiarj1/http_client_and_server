#!/bin/bash

PYTHON=/usr/local/python3/bin/python3
TIMEOUT=5 # seconds
KILL_TIMEOUT=10 # seconds


if [ -z $1 ]
then
  echo "Usage: test.sh url"
  echo "Example: ./test.sh http://www.example.com"
  exit 1
else
  URL=$1
fi


## PARSE CMD LINE ARGS ##

if [ -f "http_cli" ]
then
  CMD="./http_cli"
elif [ -f "http_cli.py" ]
then
  CMD="$PYTHON http_cli.py"
else
  echo "ERROR: Required files do not exist!"
  ls
  exit 1
fi


## TESTS BEGIN HERE ##

URL=$1

echo ========================================
echo === $URL ===
echo

timeout -k $KILL_TIMEOUT $TIMEOUT $CMD $URL > myfile.test

if [ $? -eq 124 ]
then
  echo
  echo "--- Test timed out after $TIMEOUT sec. ---"

else
  echo
  echo "--- Test finished with exit code $? ---"
  echo
  curl $URL > curlfile.test
  echo ----- Diff -----
  diff curlfile.test myfile.test
  echo ----------------
fi
rm -f myfile.test curlfile.test

