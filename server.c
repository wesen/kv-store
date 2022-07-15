#include <stdio.h>
#include <zmq.h>
#include <unistd.h>
#include <assert.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, "tcp://*:5555");
    assert(rc == 0);

    for (;;) {
        char buffer[10];
        zmq_recv(responder, buffer, 10, 0);
        printf("Received hello\n");
        sleep(1);
        zmq_send(responder, "World", 5, 0);
    }

    printf("Hello world\n");
    return 0;
}
