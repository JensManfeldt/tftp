headers = server.h connection.h packet_types.h packet_parsing.h

run: clean build
	./run.sh

build: main server connection packet_parsing
	cc -o build/main build/main.o build/server.o build/connection.o build/packet_parsing.o $(headers)

main :
	cc -c main.c -o build/main.o

server :
	cc -c server.c -o build/server.o

connection :
	cc -c connection.c -o build/connection.o

packet_parsing :
	cc -c packet_parsing.c -o build/packet_parsing.o


test : build
	./test/test.sh

clean:
	rm -f build/*
