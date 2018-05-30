/*
 * BlueALSA - bluez-a2dp.c
 * Copyright (c) 2016-2018 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#include "bluez-a2dp.h"

static const a2dp_sbc_t a2dp_sbc = {
	.frequency =
		SBC_SAMPLING_FREQ_16000 |
		SBC_SAMPLING_FREQ_32000 |
		SBC_SAMPLING_FREQ_44100 |
		SBC_SAMPLING_FREQ_48000,
	.channel_mode =
		SBC_CHANNEL_MODE_MONO |
		SBC_CHANNEL_MODE_DUAL_CHANNEL |
		SBC_CHANNEL_MODE_STEREO |
		SBC_CHANNEL_MODE_JOINT_STEREO,
	.block_length =
		SBC_BLOCK_LENGTH_4 |
		SBC_BLOCK_LENGTH_8 |
		SBC_BLOCK_LENGTH_12 |
		SBC_BLOCK_LENGTH_16,
	.subbands =
		SBC_SUBBANDS_4 |
		SBC_SUBBANDS_8,
	.allocation_method =
		SBC_ALLOCATION_SNR |
		SBC_ALLOCATION_LOUDNESS,
	.min_bitpool = SBC_MIN_BITPOOL,
	.max_bitpool = SBC_MAX_BITPOOL,
};

static const a2dp_mpeg_t a2dp_mpeg = {
	.layer =
		MPEG_LAYER_MP1 |
		MPEG_LAYER_MP2 |
		MPEG_LAYER_MP3,
	.crc = 1,
	.channel_mode =
		MPEG_CHANNEL_MODE_MONO |
		MPEG_CHANNEL_MODE_DUAL_CHANNEL |
		MPEG_CHANNEL_MODE_STEREO |
		MPEG_CHANNEL_MODE_JOINT_STEREO,
	.mpf = 1,
	.frequency =
		MPEG_SAMPLING_FREQ_16000 |
		MPEG_SAMPLING_FREQ_22050 |
		MPEG_SAMPLING_FREQ_24000 |
		MPEG_SAMPLING_FREQ_32000 |
		MPEG_SAMPLING_FREQ_44100 |
		MPEG_SAMPLING_FREQ_48000,
	.bitrate =
		MPEG_BIT_RATE_VBR |
		MPEG_BIT_RATE_320000 |
		MPEG_BIT_RATE_256000 |
		MPEG_BIT_RATE_224000 |
		MPEG_BIT_RATE_192000 |
		MPEG_BIT_RATE_160000 |
		MPEG_BIT_RATE_128000 |
		MPEG_BIT_RATE_112000 |
		MPEG_BIT_RATE_96000 |
		MPEG_BIT_RATE_80000 |
		MPEG_BIT_RATE_64000 |
		MPEG_BIT_RATE_56000 |
		MPEG_BIT_RATE_48000 |
		MPEG_BIT_RATE_40000 |
		MPEG_BIT_RATE_32000 |
		MPEG_BIT_RATE_FREE,
};

static const a2dp_aac_t a2dp_aac = {
	.object_type =
		/* NOTE: AAC Long Term Prediction and AAC Scalable are
		 *       not supported by the FDK-AAC library. */
		AAC_OBJECT_TYPE_MPEG2_AAC_LC |
		AAC_OBJECT_TYPE_MPEG4_AAC_LC,
	AAC_INIT_FREQUENCY(
		AAC_SAMPLING_FREQ_8000 |
		AAC_SAMPLING_FREQ_11025 |
		AAC_SAMPLING_FREQ_12000 |
		AAC_SAMPLING_FREQ_16000 |
		AAC_SAMPLING_FREQ_22050 |
		AAC_SAMPLING_FREQ_24000 |
		AAC_SAMPLING_FREQ_32000 |
		AAC_SAMPLING_FREQ_44100 |
		AAC_SAMPLING_FREQ_48000 |
		AAC_SAMPLING_FREQ_64000 |
		AAC_SAMPLING_FREQ_88200 |
		AAC_SAMPLING_FREQ_96000)
	.channels =
		AAC_CHANNELS_1 |
		AAC_CHANNELS_2,
	.vbr = 1,
	AAC_INIT_BITRATE(0xFFFF)
};

static const a2dp_aptx_t a2dp_aptx = {
	.info.vendor_id = APTX_VENDOR_ID,
	.info.codec_id = APTX_CODEC_ID,
	.channel_mode =
		/* NOTE: Used apt-X library does not support
		 *       single channel (mono) mode. */
		APTX_CHANNEL_MODE_STEREO,
	.frequency =
		APTX_SAMPLING_FREQ_16000 |
		APTX_SAMPLING_FREQ_32000 |
		APTX_SAMPLING_FREQ_44100 |
		APTX_SAMPLING_FREQ_48000,
};

static const a2dp_ldac_t a2dp_ldac = {
	.info.vendor_id = LDAC_VENDOR_ID,
	.info.codec_id = LDAC_CODEC_ID,
	.channel_mode =
		LDAC_CHANNEL_MODE_MONO |
		LDAC_CHANNEL_MODE_DUAL_CHANNEL |
		LDAC_CHANNEL_MODE_STEREO,
	.frequency =
		/* NOTE: Used LDAC library does not support
		 *       frequencies higher than 96 kHz. */
		LDAC_SAMPLING_FREQ_44100 |
		LDAC_SAMPLING_FREQ_48000 |
		LDAC_SAMPLING_FREQ_88200 |
		LDAC_SAMPLING_FREQ_96000,
};

static const struct bluez_a2dp_codec a2dp_codec_source_sbc = {
	.dir = BLUEZ_A2DP_SOURCE,
	.id = A2DP_CODEC_SBC,
	.cfg = &a2dp_sbc,
	.cfg_size = sizeof(a2dp_sbc),
};

static const struct bluez_a2dp_codec a2dp_codec_sink_sbc = {
	.dir = BLUEZ_A2DP_SINK,
	.id = A2DP_CODEC_SBC,
	.cfg = &a2dp_sbc,
	.cfg_size = sizeof(a2dp_sbc),
};

static const struct bluez_a2dp_codec a2dp_codec_source_mpeg = {
	.dir = BLUEZ_A2DP_SOURCE,
	.id = A2DP_CODEC_MPEG12,
	.cfg = &a2dp_mpeg,
	.cfg_size = sizeof(a2dp_mpeg),
};

static const struct bluez_a2dp_codec a2dp_codec_sink_mpeg = {
	.dir = BLUEZ_A2DP_SINK,
	.id = A2DP_CODEC_MPEG12,
	.cfg = &a2dp_mpeg,
	.cfg_size = sizeof(a2dp_mpeg),
};

static const struct bluez_a2dp_codec a2dp_codec_source_aac = {
	.dir = BLUEZ_A2DP_SOURCE,
	.id = A2DP_CODEC_MPEG24,
	.cfg = &a2dp_aac,
	.cfg_size = sizeof(a2dp_aac),
};

static const struct bluez_a2dp_codec a2dp_codec_sink_aac = {
	.dir = BLUEZ_A2DP_SINK,
	.id = A2DP_CODEC_MPEG24,
	.cfg = &a2dp_aac,
	.cfg_size = sizeof(a2dp_aac),
};

static const struct bluez_a2dp_codec a2dp_codec_source_aptx = {
	.dir = BLUEZ_A2DP_SOURCE,
	.id = A2DP_CODEC_VENDOR_APTX,
	.cfg = &a2dp_aptx,
	.cfg_size = sizeof(a2dp_aptx),
};

static const struct bluez_a2dp_codec a2dp_codec_sink_aptx = {
	.dir = BLUEZ_A2DP_SINK,
	.id = A2DP_CODEC_VENDOR_APTX,
	.cfg = &a2dp_aptx,
	.cfg_size = sizeof(a2dp_aptx),
};

static const struct bluez_a2dp_codec a2dp_codec_source_ldac = {
	.dir = BLUEZ_A2DP_SOURCE,
	.id = A2DP_CODEC_VENDOR_LDAC,
	.cfg = &a2dp_ldac,
	.cfg_size = sizeof(a2dp_ldac),
};

static const struct bluez_a2dp_codec a2dp_codec_sink_ldac = {
	.dir = BLUEZ_A2DP_SINK,
	.id = A2DP_CODEC_VENDOR_LDAC,
	.cfg = &a2dp_ldac,
	.cfg_size = sizeof(a2dp_ldac),
};

static const struct bluez_a2dp_codec *a2dp_codecs[] = {
#if ENABLE_LDAC
	&a2dp_codec_source_ldac,
#endif
#if ENABLE_APTX
	&a2dp_codec_source_aptx,
#endif
#if ENABLE_AAC
	&a2dp_codec_source_aac,
	&a2dp_codec_sink_aac,
#endif
#if ENABLE_MPEG
	&a2dp_codec_source_mpeg,
	&a2dp_codec_sink_mpeg,
#endif
	&a2dp_codec_source_sbc,
	&a2dp_codec_sink_sbc,
	NULL,
};

struct bluez_a2dp_channel_mode bluez_a2dp_channels_sbc[4] = {
	{ BLUEZ_A2DP_CHM_JOINT_STEREO, SBC_CHANNEL_MODE_JOINT_STEREO },
	{ BLUEZ_A2DP_CHM_STEREO, SBC_CHANNEL_MODE_STEREO },
	{ BLUEZ_A2DP_CHM_DUAL_CHANNEL, SBC_CHANNEL_MODE_DUAL_CHANNEL },
	{ BLUEZ_A2DP_CHM_MONO, SBC_CHANNEL_MODE_MONO }};
struct bluez_a2dp_frequency bluez_a2dp_frequencies_sbc[4] = {
	{ 48000, SBC_SAMPLING_FREQ_48000 },
	{ 44100, SBC_SAMPLING_FREQ_44100 },
	{ 32000, SBC_SAMPLING_FREQ_32000 },
	{ 16000, SBC_SAMPLING_FREQ_16000 }};

#if ENABLE_MPEG
struct bluez_a2dp_channel_mode bluez_a2dp_channels_mpeg[4] = {
	{ BLUEZ_A2DP_CHM_JOINT_STEREO, MPEG_CHANNEL_MODE_JOINT_STEREO },
	{ BLUEZ_A2DP_CHM_STEREO, MPEG_CHANNEL_MODE_STEREO },
	{ BLUEZ_A2DP_CHM_DUAL_CHANNEL, MPEG_CHANNEL_MODE_DUAL_CHANNEL },
	{ BLUEZ_A2DP_CHM_MONO, MPEG_CHANNEL_MODE_MONO }};
struct bluez_a2dp_frequency bluez_a2dp_frequencies_mpeg[6] = {
	{ 48000, MPEG_SAMPLING_FREQ_48000 },
	{ 44100, MPEG_SAMPLING_FREQ_44100 },
	{ 32000, MPEG_SAMPLING_FREQ_32000 },
	{ 24000, MPEG_SAMPLING_FREQ_24000 },
	{ 22050, MPEG_SAMPLING_FREQ_22050 },
	{ 16000, MPEG_SAMPLING_FREQ_16000 }};
#endif

#if ENABLE_AAC
struct bluez_a2dp_channel_mode bluez_a2dp_channels_aac[2] = {
	{ BLUEZ_A2DP_CHM_STEREO, AAC_CHANNELS_2 },
	{ BLUEZ_A2DP_CHM_MONO, AAC_CHANNELS_1 }};
struct bluez_a2dp_frequency bluez_a2dp_frequencies_aac[12] = {
	{ 96000, AAC_SAMPLING_FREQ_96000 },
	{ 88200, AAC_SAMPLING_FREQ_88200 },
	{ 64000, AAC_SAMPLING_FREQ_64000 },
	{ 48000, AAC_SAMPLING_FREQ_48000 },
	{ 44100, AAC_SAMPLING_FREQ_44100 },
	{ 32000, AAC_SAMPLING_FREQ_32000 },
	{ 24000, AAC_SAMPLING_FREQ_24000 },
	{ 22050, AAC_SAMPLING_FREQ_22050 },
	{ 16000, AAC_SAMPLING_FREQ_16000 },
	{ 12000, AAC_SAMPLING_FREQ_12000 },
	{ 11025, AAC_SAMPLING_FREQ_11025 },
	{ 8000, AAC_SAMPLING_FREQ_8000 }};
#endif

#if ENABLE_APTX
struct bluez_a2dp_channel_mode bluez_a2dp_channels_aptx[1] = {
	{ BLUEZ_A2DP_CHM_STEREO, APTX_CHANNEL_MODE_STEREO }};
struct bluez_a2dp_frequency bluez_a2dp_frequencies_aptx[4] = {
	{ 48000, APTX_SAMPLING_FREQ_48000 },
	{ 44100, APTX_SAMPLING_FREQ_44100 },
	{ 32000, APTX_SAMPLING_FREQ_32000 },
	{ 16000, APTX_SAMPLING_FREQ_16000 }};
#endif

#if ENABLE_LDAC
struct bluez_a2dp_channel_mode bluez_a2dp_channels_ldac[3] = {
	{ BLUEZ_A2DP_CHM_STEREO, LDAC_CHANNEL_MODE_STEREO },
	{ BLUEZ_A2DP_CHM_DUAL_CHANNEL, LDAC_CHANNEL_MODE_DUAL_CHANNEL },
	{ BLUEZ_A2DP_CHM_MONO, LDAC_CHANNEL_MODE_MONO }};
struct bluez_a2dp_frequency bluez_a2dp_frequencies_ldac[4] = {
	{ 96000, LDAC_SAMPLING_FREQ_96000 },
	{ 88200, LDAC_SAMPLING_FREQ_88200 },
	{ 48000, LDAC_SAMPLING_FREQ_48000 },
	{ 44100, LDAC_SAMPLING_FREQ_44100 }};
#endif

const struct bluez_a2dp_codec **bluez_a2dp_codecs = a2dp_codecs;
