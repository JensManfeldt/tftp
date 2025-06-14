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

run_client : build_client
	./build/client -a 127.0.0.1 -p 8888 -f adasd

build_client :
	$(compiler) clients/client_retransmission.c -o build/client

test : build build_client
	./test/test.sh

clean:
	rm -f build/*
