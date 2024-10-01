#include "common.h"
#include <zmq.h>
#include <unistd.h>

int main() {
    // Initialize ZeroMQ context
    void *context = zmq_ctx_new();

    // Create a SUB (Subscriber) socket to receive messages from clients
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_bind(subscriber, "tcp://*:5555");  // Listen to clients on port 5555
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);  // Subscribe to all messages

    // Create a PUB (Publisher) socket to broadcast messages to clients
    void *publisher = zmq_socket(context, ZMQ_PUB);
    zmq_bind(publisher, "tcp://*:5556");  // Broadcast to clients on port 5556

    char* buffer = calloc(1, BUFFER_SIZE + 1);
    while (1) {

		zmq_recv(subscriber, buffer, BUFFER_SIZE, 0);

        zmq_send(publisher, buffer, BUFFER_SIZE, 0);
    }

    // Clean up
    zmq_close(subscriber);
    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}

