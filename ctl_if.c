#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#define CONTROL_SOCKET_NAME	"/tmp/control_socket"
int	control_socket = -1;

int	tx_power = 0;		/* Global tx_power variable. */


static int setup_socket (void)
{
	struct sockaddr_un	addr;

	/* Create socket file handle. */
	control_socket = socket (AF_UNIX, SOCK_DGRAM, 0);
	if (control_socket < 0) {
		perror ("socket(AF_UNIX, SOCK_STREAM, 0):");
		return -1;
	}

	/* Bind socket to file name (remove file first to avoid errors). */
	memset (&addr, 0, sizeof (addr));
	addr.sun_family = AF_UNIX;
	strncpy (addr.sun_path, CONTROL_SOCKET_NAME, sizeof (addr.sun_path) - 1);
	unlink (addr.sun_path);
	if (bind (control_socket, (struct sockaddr *)&addr, sizeof (addr)) < 0) {
		perror ("bind (control_socket,,,):");
		close (control_socket);
		control_socket = -1;
		return -1;
	}

	return 0;
}


static int poll_socket (void)
{
	fd_set		read_fds;
	int		ret;
	struct timeval	timeout;

	if (control_socket < 0) {
		/* Socket is not yet open. */
		setup_socket ();
		if (control_socket < 0) {
			return 0;		/* Unable to open socket. */
		}
	}

	FD_ZERO (&read_fds);
	FD_SET (control_socket, &read_fds);
	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;
	ret = select (control_socket + 1, &read_fds, NULL, NULL, &timeout);
	if (ret > 0 && FD_ISSET (control_socket, &read_fds)) {
		/* Data received. */
		unsigned char		cmd_buf [100];
		struct sockaddr_un	from;
		socklen_t		from_len = sizeof (from);

		ret = recvfrom (control_socket, &cmd_buf, sizeof (cmd_buf), 0, (struct sockaddr *)&from, &from_len);
		if (ret < 0) {
			/* Problem with the file. */
			close (control_socket);
			control_socket = -1;
			return -1;
		}

		/* Parse the received message. */
		switch (cmd_buf [0]) {
			case 0:		/* Poll for tx_power value. */
				cmd_buf [1] = tx_power;

				ret = sendto (control_socket, &cmd_buf, 2, 0, (struct sockaddr *)&from, from_len);
				/* If the reply fails too bad. Ignore it. */
				break;
			case 1:		/* Set tx_power value. */
				tx_power = cmd_buf [1];

				/* Return the same message as acknowledgement. */
				ret = sendto (control_socket, &cmd_buf, 2, 0, (struct sockaddr *)&from, from_len);
				/* If the reply fails too bad. Ignore it. */
				break;
		}
	}

	return 0;
}

#if 0
int main (int argc, char **argv)
{
	while (1) {
		poll_socket ();
		usleep (100000);
	}

	return 0;
}
#endif

/* returns 1 if there is new tx_power */
int ctl_if_poll(void)
{
    int current_power = tx_power;

    if (poll_socket() < 0)
        return 0;
    else
        return current_power != tx_power;
}

int ctl_if_get_tx_power(void)
{
    return tx_power;
}

void ctl_if_set_tx_power(int power)
{
    tx_power = power;
}
