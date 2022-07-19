#include <stdio.h>
#include <zmq.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>

#include "z_helpers.h"

int main(void) {
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    int rc = zmq_connect(requester, "tcp://localhost:5555");
    assert(rc == 0);

    for (int request_nbr = 0; request_nbr < 10; request_nbr++) {
        char buffer[10];
        printf("Sending hello %d...\n", request_nbr);
        zmq_send(requester, "Hello", 5, 0);
        int size = zmq_recv(requester, buffer, 9, 0);
        if (size < sizeof(buffer)) {
          buffer[size] = 0;
          printf("Received World %d: %s\n", request_nbr, buffer);
        } else {
            printf("Illegal string size %d\n", size);
        }
        sleep(1);
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
