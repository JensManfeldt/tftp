compiler = gcc -Wall -Werror
headers = server.h connection.h packet_types.h packet_parsing.h


run: clean build
	./run.sh

build: main server connection packet_parsing
	$(compiler) -o build/main build/main.o build/server.o build/connection.o build/packet_parsing.o $(headers)

main :
	$(compiler) -c main.c -o build/main.o

server :
	$(compiler) -c server.c -o build/server.o

connection :
	$(compiler) -c connection.c -o build/connection.o

packet_parsing :
	$(compiler) -c packet_parsing.c -o build/packet_parsing.o


test : build
	./test/test.sh

clean:
	rm -f build/*
