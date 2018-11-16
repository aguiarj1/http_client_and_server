#!/bin/bash

./test.sh

./test.sh http/badurl.com:blah

./test.sh blah_no.colons-or

./test.sh http://flask.pocoo.org/docs/1.0/index.html

./test.sh http://flask.pocoo.org/

./test.sh http://www.example.com

./test.sh http://jinja.pocoo.org/docs/2.10

./test.sh http://flask.pocoo.org/notfound.html

./test.sh http://flask.pocoo.org/static/logo/flask.png

./test.sh http://ichef.bbci.co.uk/wwfeatures/976_549/images/live/p0/34/pb/p034pb0g.jpg
