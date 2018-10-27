CLIENT = http_cli

all: $(CLIENT)

$(CLIENT): $(CLIENT).cpp
	g++ -g -std=c++11 -g -Wall -Werror -o $@ $^

clean:
	rm -f *.o $(CLIENT)
