CLIENT = http_cli
SERVER = http_svr


all: $(CLIENT) $(SERVER)

$(CLIENT): $(CLIENT).cpp
	g++ -std=c++11 -g -Wall -Werror -o $@ $^


$(SERVER): $(SERVER).cpp
	g++ -std=c++11 -g -Wall -Werror -o $@ $^



clean:
	rm -f *.o $(CLIENT)
