/*
 * BlueALSA - bluez-a2dp.h
 * Copyright (c) 2016-2018 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#ifndef BLUEALSA_BLUEZA2DP_H_
#define BLUEALSA_BLUEZA2DP_H_

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stddef.h>

#include "a2dp-codecs.h"

enum bluez_a2dp_chm {
	BLUEZ_A2DP_CHM_MONO = 0,
	/* fixed bit-rate for each channel */
	BLUEZ_A2DP_CHM_DUAL_CHANNEL,
	/* channel bits allocated dynamically */
	BLUEZ_A2DP_CHM_STEREO,
	/* L+R (mid) and L-R (side) encoding */
	BLUEZ_A2DP_CHM_JOINT_STEREO,
};

enum bluez_a2dp_dir {
	BLUEZ_A2DP_SOURCE,
	BLUEZ_A2DP_SINK,
};

struct bluez_a2dp_channel_mode {
	enum bluez_a2dp_chm channel_mode;
	uint16_t value;
};

struct bluez_a2dp_frequency {
	int frequency;
	uint16_t value;
};

struct bluez_a2dp_codec {
	enum bluez_a2dp_dir dir;
	uint16_t id;
	const void *cfg;
	size_t cfg_size;
};

struct bluez_a2dp_channel_mode bluez_a2dp_channels_sbc[4];
struct bluez_a2dp_frequency bluez_a2dp_frequencies_sbc[4];
#if ENABLE_MPEG
struct bluez_a2dp_channel_mode bluez_a2dp_channels_mpeg[4];
struct bluez_a2dp_frequency bluez_a2dp_frequencies_mpeg[6];
#endif
#if ENABLE_AAC
struct bluez_a2dp_channel_mode bluez_a2dp_channels_aac[2];
struct bluez_a2dp_frequency bluez_a2dp_frequencies_aac[12];
#endif
#if ENABLE_APTX
struct bluez_a2dp_channel_mode bluez_a2dp_channels_aptx[1];
struct bluez_a2dp_frequency bluez_a2dp_frequencies_aptx[4];
#endif

/* NULL-terminated list of available A2DP codecs */
const struct bluez_a2dp_codec **bluez_a2dp_codecs;

#endif
