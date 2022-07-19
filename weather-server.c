#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "z_helpers.h"
#include "zmq.h"

int main(void) {
    void *context = zmq_ctx_new();
    void *publisher = zmq_socket(context, ZMQ_PUB);

    int rc = zmq_bind(publisher, "tcp://*:5556/");
    assert(rc == 0);

    srandom((unsigned)time(NULL));
    uint64_t cnt = 0;
    for (;;) {
        int zipcode = randof(1000000);
        int temperature = randof(215) - 80;
        int relhumidity = randof(50) + 10;
        cnt++;
        if (cnt > 1000000) {
            printf("1M sent!\n");
            cnt = 0;
        }

        char update[20];
        snprintf(update, sizeof(update), "%05d %d %d", zipcode, temperature, relhumidity);
        s_send(publisher, update);
    }
    zmq_close(publisher);
    zmq_ctx_destroy(context);

    return 0;
}
