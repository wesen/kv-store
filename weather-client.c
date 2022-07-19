#include "z_helpers.h"
#include "zmq.h"
#include <inttypes.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("Collecting updates from weather server\n");
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    int32_t rc = zmq_connect(subscriber, "tcp://localhost:5556");
    pid_t pid = getpid();
    assert(rc == 0);

    const char *filter = (argc > 1) ? argv[1] : "10001 ";
    rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter));
    assert(rc == 0);

    int64_t total_temp = 0;
    int32_t update_nbr;
    for (update_nbr = 0; update_nbr < 100; update_nbr++) {
        // XXX check for NULL
        char *s = s_recv(subscriber);

        int32_t zipcode, temperature, relhumidity;
        sscanf(s, "%d %d %d", &zipcode, &temperature, &relhumidity);
        total_temp += temperature;
        printf("%05d - reading number %d - %d %d %d\n", pid, update_nbr, zipcode, temperature, relhumidity);
        free(s);
    }

    printf("Average tempereature for zipcode '%s' was %dF\n",
           filter, (int)(total_temp / update_nbr));

    zmq_close(subscriber);
    zmq_ctx_destroy(context);

    return 0;
}
