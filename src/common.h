#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#define PORT 8888
#define BUFFER_SIZE 100
#define SERVER_IP "127.0.0.1"

#ifndef LOG
#define LOG(format, ...) printf(format, ##__VA_ARGS__)
#endif

#define  INFO(format, ...) LOG("[INFO]  " format "\n", ##__VA_ARGS__)
#define  WARN(format, ...) LOG("[WARN]  " format "\n", ##__VA_ARGS__)
#define ERROR(format, ...) LOG("[ERROR] " format "\n", ##__VA_ARGS__)
#define DEBUG(format, ...) LOG("[DEBUG] " format "\n", ##__VA_ARGS__)

void sleep_ms(unsigned long milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000000000ul;            // whole seconds
    ts.tv_nsec = (milliseconds % 1000000000ul) * 1000000;  // remainder, in nanoseconds
    nanosleep(&ts, NULL);
}

typedef enum {
	READING = 'R',
	WAITING = '\0',
	WRITING = 'W',
	SENDER = 'S',
	RECIEVER = 'G'
} Client_State;

const char* state_name(Client_State state) {
	switch (state) {
		case  READING: return  "READING";
		case  WAITING: return  "WAITING";
		case  WRITING: return  "WRITING";
		case   SENDER: return   "SENDER";
		case RECIEVER: return "RECIEVER";
	}
	return "UNDEFINED";
}
#ifdef ERROR
#define ERROR_UNKNOWN(state) ERROR("unknown status: '%s' '%c' (%i)", state_name(state), state, state)
#endif
