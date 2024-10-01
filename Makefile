CC=clang
WORKING_DIR=.
CFLAGS=-Wall -Wextra -ggdb -std=c11 -D=_GNU_SOURCE -O3
CFLAGS_S=$(CFLAGS)
CFLAGS_C=$(CFLAGS)
INCLUDES=-I$(WORKING_DIR)/include -I/usr/include
LIBS=-L./lib/dynamic/ -lzmq
LIBS_S=$(LIBS)
LIBS_C=$(LIBS) -lraylib -lm

BUILD_LIBS=-L./lib/static/ -l:libzmq.a
BUILD_LIBS_S=$(BUILD_LIBS)
BUILD_LIBS_C=$(BUILD_LIBS) -l:libraylib.a -lpthread -lc -Wl,-Bdynamic -l:libm.so
STATIC=-Wl,-Bstatic -static-libgcc -static-libstdc++ -no-pie

all: client server

build-all: client-build server-build

client: ./src/client.c
	$(CC) $(CFLAGS_C) $(INCLUDES) -o $(WORKING_DIR)/client $(WORKING_DIR)/src/client.c $(LIBS_C)

client-build: ./src/client.c
	$(CC) $(CFLAGS_C) $(INCLUDES) -c -o $(WORKING_DIR)/client.o -c $(WORKING_DIR)/src/client.c && \
	g++ -o $(WORKING_DIR)/client $(WORKING_DIR)/client.o $(BUILD_LIBS_C) $(STATIC) && \
	rm $(WORKING_DIR)/client.o

server: ./src/server.c
	$(CC) $(CFLAGS_S) $(INCLUDES) -o $(WORKING_DIR)/server $(WORKING_DIR)/src/server.c $(LIBS_S)

server-build: ./src/server.c
	$(CC) $(CFLAGS_S) $(INCLUDES) -c -o $(WORKING_DIR)/server.o -c $(WORKING_DIR)/src/server.c && \
	g++ -o $(WORKING_DIR)/server $(WORKING_DIR)/server.o $(BUILD_LIBS_S) $(STATIC) && \
	rm $(WORKING_DIR)/server.o
