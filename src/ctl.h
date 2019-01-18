/*
 * BlueALSA - ctl.h
 * Copyright (c) 2016-2018 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#ifndef BLUEALSA_CTL_H_
#define BLUEALSA_CTL_H_

#include <bluealsa/ctl-proto.h>

/* Indexes of special file descriptors in the poll array. */
#define CTL_IDX_SRV 0
#define CTL_IDX_EVT 1
#define __CTL_IDX_MAX 2

int bluealsa_ctl_thread_init(void);
void bluealsa_ctl_free(void);

int bluealsa_ctl_send_event(enum ba_event event, const bdaddr_t *addr, uint8_t type);

#endif
