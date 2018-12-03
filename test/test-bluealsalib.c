/*
 * test-bluealsalib.c
 * Copyright (c) 2018 Thierry Bultel
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

// TODO monitor all available interfaces
const char * ba_interface = "hci0";

#include <bluealsa/bluealsa.h>
#include <bluealsa/log.h>
#include <pthread.h>
#include <errno.h>
#include <poll.h>


static void * monitor_thread_entry(void* arg) {
	debug("...");

	int ba_fd, ba_event_fd;
	enum ba_event transport_mask = BA_EVENT_TRANSPORT_ADDED | BA_EVENT_TRANSPORT_CHANGED|BA_EVENT_TRANSPORT_REMOVED;

	if ((ba_fd = bluealsa_open(ba_interface)) == -1) {
		error("BlueALSA connection failed: %s", strerror(errno));
		goto fail;
	}

	if ((ba_event_fd = bluealsa_open(ba_interface)) == -1) {
		error("BlueALSA connection failed: %s", strerror(errno));
		goto fail;
	}

	if (bluealsa_subscribe(ba_event_fd, transport_mask) == -1) {
		error("BlueALSA subscription failed: %s", strerror(errno));
		goto fail;
	}

goto init;

	while (true) {

		struct ba_msg_event event;
		struct ba_msg_transport *transports;
		ssize_t ret;
		size_t i;

		struct pollfd pfds[] = {{ ba_event_fd, POLLIN, 0 }};
		if (poll(pfds, ARRAYSIZE(pfds), -1) == -1 && errno == EINTR)
			continue;

		while ((ret = recv(ba_event_fd, &event, sizeof(event), MSG_DONTWAIT)) == -1 && errno == EINTR)
			continue;
		if (ret != sizeof(event)) {
			error("Couldn't read event: %s", strerror(ret == -1 ? errno : EBADMSG));
			goto fail;
		}

init:
		debug("Fetching available transports");
		if ((ret = bluealsa_get_transports(ba_fd, &transports)) == -1) {
			error("Couldn't get transports: %s", strerror(errno));
			goto fail;
		}

		debug("Got %d transports", ret);

		for (int ix=0; ix<ret; ix++) {
			char addr[18];
			struct ba_msg_transport * transport = &transports[ix];
			ba2str(&transport->addr, addr);
			info("Transport %d: type %d, dev %s", ix, transport->type, addr);
		}

	}

fail:
	info("exit");
	return NULL;
}

int main(int argc, char * argv[]) {
	printf("%s... !\n", argv[0]);

	pthread_t monitor;
	if (pthread_create(&monitor, NULL, monitor_thread_entry, NULL) == -1) {
		debug("failed to create the monitor thread");
		goto fail;
	}
	pthread_join(monitor, NULL);
fail:
	return 0;
}
