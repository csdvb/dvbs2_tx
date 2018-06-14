#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <sys/select.h>

#define CONTROL_SOCKET_NAME	"/tmp/control_socket"

int main (int argc, char **argv)
{
    struct sockaddr_un 	my_addr;
    unsigned char	buf [100];
    int     		client_socket;
    ssize_t 		rc;
    struct sockaddr_un 	to_addr;
    socklen_t   	to_len;
    struct timeval  	timeout;
    fd_set          	read_fds;

    client_socket = socket (AF_UNIX, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror ("socket (AF_UNIX, SOCK_DGRAM, 0):");
        return 1;
    }

    /* Setup a return address so the server can return a reply. */
    memset (&my_addr, 0, sizeof (my_addr));
    my_addr.sun_family = AF_UNIX;
    snprintf (my_addr.sun_path, sizeof (my_addr.sun_path) - 1, "/tmp/%d-return", getpid ());
    unlink (my_addr.sun_path);
    if (bind (client_socket, (struct sockaddr *)&my_addr, sizeof (my_addr)) < 0) {
        perror ("bind (client_socket,,,):");
        return 1;
    }

    /* Send message to server. */
    memset (&to_addr, 0, sizeof (to_addr));
    to_addr.sun_family = AF_UNIX;
    snprintf (to_addr.sun_path, sizeof (to_addr.sun_path) - 1, CONTROL_SOCKET_NAME); 
    to_len = sizeof (to_addr);

    if (argc >= 2) {
        /* Send the provided numerical value to the server. */
	buf [0] = 1;	/* Set command. */
	buf [1] = atoi (argv [1]);
    } else {
        /* Poll the server. */
	buf [0] = 0;	/* Poll command. */
    }

    rc = sendto (client_socket, buf, 2, 0,
                 (struct sockaddr *)&to_addr, to_len);
    if (rc < 0) {
        perror ("sendto (client_sock,,,):");
        return 1;
    }

    /* Wait for a reply - but only for 2 seconds. */
    timeout.tv_sec  = 2;
    timeout.tv_usec = 0;
    FD_ZERO (&read_fds);
    FD_SET (client_socket, &read_fds);
    rc = select (client_socket + 1, &read_fds, NULL, NULL, &timeout);
    if (rc == 1) {
        /* Got something. */
        char buf [100];
        struct sockaddr_un  from;
        socklen_t           from_len = sizeof (from);
	int		    x;

        rc = recvfrom (client_socket, &buf, sizeof (buf), 0,
                       (struct sockaddr *)&from, &from_len);

        printf ("Modtaget ");
	for (x = 0; x < rc; x++) {
	    printf (" %02X", buf [x]);
	}
	printf ("\n");
    } else {
        printf ("Timeout waiting for data\n");
    }

    close (client_socket);
    unlink (my_addr.sun_path);

    return 0;
}
