/*
 * BlueALSA - bluealsa.h
 * Copyright (c) 2018 Thierry Bultel
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#ifndef BLUEALSA_H
#define BLUEALSA_H

#include <bluealsa/ctl-client.h>
#include <bluealsa/defs.h>

typedef int (*transport_update_cb) (struct ba_msg_transport *transports);

extern int bluelsa_register_transport_update_cb(const char * interfance, transport_update_cb cb);

#endif
