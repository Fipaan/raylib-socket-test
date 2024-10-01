#include "common.h"
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#define ID_SIZE 4
#define USERNAME_SIZE 8
#define MSG_SIZE (BUFFER_SIZE - ID_SIZE - 2)
void *context, *subscriber, *publisher;
typedef struct {
	char id[ID_SIZE + 1];
	char msg[MSG_SIZE + 1];
} Message;
Message tmp_msg = {0};
bool reading_mode, username_reading_mode;
char recieved_username[USERNAME_SIZE + 1];
clock_t init_time;
struct Me {
	const char id[ID_SIZE + 1];
	char username[USERNAME_SIZE + 1];
} me;

const char* valid_chars_id = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";

int randbt(int ibottom, int etop) {
	return ibottom + rand() % (etop - ibottom);
}

void socket_init() {
	// Initialize ZeroMQ context
	context = zmq_ctx_new();

	// Create a SUB (Subscriber) socket to receive broadcasted messages from the broker
	subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, "tcp://localhost:5556");  // Connect to broker's PUB socket on port 5556
	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);  // Subscribe to all messages

	// Create a PUB (Publisher) socket to send messages to the broker
	publisher = zmq_socket(context, ZMQ_PUB);
	zmq_connect(publisher, "tcp://localhost:5555");  // Connect to broker's SUB socket on port 5555
	srand(time(NULL));
	for(size_t i = 0; i < ID_SIZE; ++i) {
		*(char*)(me.id + i) = valid_chars_id[randbt(0, strlen(valid_chars_id))];
	}
}

void hello_msg(const char* msg) {
	char* sending_msg;
	int ignore = asprintf(&sending_msg, "h%s %s", me.id, msg);
	(void) ignore;
	sending_msg = realloc(sending_msg, BUFFER_SIZE);
	zmq_send(publisher, sending_msg, BUFFER_SIZE, 0);
}

void socket_loop(bool sending_mode, bool sending_username, char *const msg) {
	// Simulate sending a message when a condition is met
	if(sending_mode) {
		char* sending_msg;
		int ignore = asprintf(&sending_msg, "s%s %s", me.id, msg);
		(void) ignore;
		zmq_send(publisher, sending_msg, BUFFER_SIZE, 0);
	}
	if(sending_username) {
		hello_msg(me.username);
	}
	// Receive messages from the broker
	char tmp_recieved[BUFFER_SIZE + 1] = {0};
	int status = zmq_recv(subscriber, tmp_recieved, BUFFER_SIZE, ZMQ_DONTWAIT);
	if(status == -1 && errno == EAGAIN) {
		return;
	}
	if(*tmp_recieved == 's') {
		memcpy(tmp_msg.id, tmp_recieved + 1, ID_SIZE);
		memset(tmp_msg.msg, '\0', MSG_SIZE);
		memcpy(tmp_msg.msg, tmp_recieved + ID_SIZE + 2, MSG_SIZE);
		reading_mode = true;
	} else if(*tmp_recieved == 'h') {
		memcpy(tmp_msg.id, tmp_recieved + 1, ID_SIZE);
		memset(tmp_msg.msg, '\0', MSG_SIZE);
		memcpy(tmp_msg.msg, tmp_recieved + ID_SIZE + 2, MSG_SIZE);
		memcpy(recieved_username, tmp_msg.msg, USERNAME_SIZE + 1);
		username_reading_mode = true;
	}
}

void socket_close() {
	// Clean up
	zmq_close(subscriber);
	zmq_close(publisher);
	zmq_ctx_destroy(context);
}
