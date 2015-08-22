#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>

#include "zmq.h"

int main(int argc, char **argv)
{

    if (0 == strcmp(argv[1], "server"))
    {
        //  Socket to talk to clients
        void *context = zmq_ctx_new ();
        void *responder = zmq_socket (context, ZMQ_REP);
        int rc = zmq_bind (responder, "tcp://*:5555");
        if (!(rc == 0))
        {
            printf("zmq bind error");
            return -1;
        }

        while (1) {
            char buffer [10] = {0};
            zmq_recv (responder, buffer, 10, 0);
            printf ("Received: %s\n", buffer);
            sleep (1);          //  Do some 'work'
            zmq_send (responder, "World", 5, 0);
        }
    }
    else if (0 == strcmp(argv[1], "client"))
    {
        printf ("Connecting to server…\n");
        void *context = zmq_ctx_new ();
        void *requester = zmq_socket (context, ZMQ_REQ);
        zmq_connect (requester, "tcp://localhost:5555");

        int request_nbr;
        for (request_nbr = 0; request_nbr != 10; request_nbr++) {
            char buffer [10] = {0};
            printf ("Sending Hello %d…\n", request_nbr);
            zmq_send (requester, "Hello", 5, 0);
            zmq_recv (requester, buffer, 10, 0);
            printf ("Received: %s\n", buffer);
        }
        zmq_close (requester);
        zmq_ctx_destroy (context);        
    }

    return 0;
}
