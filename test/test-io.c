/*
 * test-io.c
 * Copyright (c) 2016-2018 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#define _GNU_SOURCE
#include <pthread.h>

#include <check.h>

#include "inc/sine.inc"
#include "../src/at.c"
#include "../src/bluealsa.c"
#include "../src/ctl.c"
#include "../src/io.c"
#include "../src/rfcomm.c"
#include "../src/transport.c"
#include "../src/utils.c"

static const a2dp_sbc_t config_sbc_44100_stereo = {
	.frequency = SBC_SAMPLING_FREQ_44100,
	.channel_mode = SBC_CHANNEL_MODE_STEREO,
	.block_length = SBC_BLOCK_LENGTH_16,
	.subbands = SBC_SUBBANDS_8,
	.allocation_method = SBC_ALLOCATION_LOUDNESS,
	.min_bitpool = SBC_MIN_BITPOOL,
	.max_bitpool = SBC_MAX_BITPOOL,
};

static const a2dp_aac_t config_aac_44100_stereo = {
	.object_type = AAC_OBJECT_TYPE_MPEG4_AAC_LC,
	AAC_INIT_FREQUENCY(AAC_SAMPLING_FREQ_44100)
	.channels = AAC_CHANNELS_2,
	.vbr = 1,
	AAC_INIT_BITRATE(0xFFFF)
};

static const a2dp_aptx_t config_aptx_44100_stereo = {
	.info.vendor_id = APTX_VENDOR_ID,
	.info.codec_id = APTX_CODEC_ID,
	.frequency = APTX_SAMPLING_FREQ_44100,
	.channel_mode = APTX_CHANNEL_MODE_STEREO,
};

static const a2dp_ldac_t config_ldac_44100_stereo = {
	.info.vendor_id = LDAC_VENDOR_ID,
	.info.codec_id = LDAC_CODEC_ID,
	.frequency = LDAC_SAMPLING_FREQ_44100,
	.channel_mode = LDAC_CHANNEL_MODE_STEREO,
};

/**
 * Helper function for timed thread join.
 *
 * This function takes the timeout value in milliseconds. */
static int pthread_timedjoin(pthread_t thread, void **retval, useconds_t usec) {

	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += (long)usec * 1000;

	/* normalize timespec structure */
	ts.tv_sec += ts.tv_nsec / (long)1e9;
	ts.tv_nsec = ts.tv_nsec % (long)1e9;

	return pthread_timedjoin_np(thread, retval, &ts);
}

/**
 * BT data generated by the encoder. */
static struct {
	uint8_t data[1024];
	size_t len;
} test_a2dp_bt_data[10];

static void test_a2dp_encoding(struct ba_transport *t, void *(*cb)(void *)) {

	int bt_fds[2];
	int pcm_fds[2];

	ck_assert_int_eq(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, bt_fds), 0);
	ck_assert_int_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, pcm_fds), 0);

	t->profile = BLUETOOTH_PROFILE_A2DP_SOURCE;
	t->state = TRANSPORT_ACTIVE;
	t->bt_fd = bt_fds[0];
	t->a2dp.pcm.fd = pcm_fds[1];

	pthread_t thread;
	pthread_create(&thread, NULL, cb, t);

	struct pollfd pfds[] = {{ bt_fds[1], POLLIN, 0 }};
	int16_t buffer[1024 * 10];
	size_t i = 0;

	snd_pcm_sine_s16le(buffer, sizeof(buffer) / sizeof(int16_t), 2, 0, 0.01);
	ck_assert_int_eq(write(pcm_fds[0], buffer, sizeof(buffer)), sizeof(buffer));

	memset(test_a2dp_bt_data, 0, sizeof(test_a2dp_bt_data));
	while (poll(pfds, ARRAYSIZE(pfds), 500) > 0) {

		char label[32];
		ssize_t len = read(bt_fds[1], buffer, t->mtu_write);

		if (i < ARRAYSIZE(test_a2dp_bt_data)) {
			memcpy(test_a2dp_bt_data[i].data, buffer, len);
			test_a2dp_bt_data[i++].len = len;
		}

		sprintf(label, "BT data [len: %3zd]", len);
		hexdump(label, buffer, len);

	}

	ck_assert_int_eq(pthread_cancel(thread), 0);
	ck_assert_int_eq(pthread_timedjoin(thread, NULL, 1e6), 0);

	close(pcm_fds[0]);
	close(bt_fds[1]);
}

static void test_a2dp_decoding(struct ba_transport *t, void *(*cb)(void *)) {

	int bt_fds[2];
	int pcm_fds[2];

	ck_assert_int_eq(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, bt_fds), 0);
	ck_assert_int_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, pcm_fds), 0);

	t->profile = BLUETOOTH_PROFILE_A2DP_SINK;
	t->state = TRANSPORT_ACTIVE;
	t->bt_fd = bt_fds[1];
	t->a2dp.pcm.fd = pcm_fds[0];

	pthread_t thread;
	pthread_create(&thread, NULL, cb, t);

	size_t i;
	for (i = 0; i < ARRAYSIZE(test_a2dp_bt_data); i++)
		if (test_a2dp_bt_data[i].len != 0)
			ck_assert_int_gt(write(bt_fds[0], test_a2dp_bt_data[i].data, test_a2dp_bt_data[i].len), 0);

	sleep(1);
	ck_assert_int_eq(pthread_cancel(thread), 0);
	ck_assert_int_eq(pthread_timedjoin(thread, NULL, 1e6), 0);

	close(pcm_fds[1]);
	close(bt_fds[0]);
}

START_TEST(test_a2dp_sbc) {

	struct ba_transport transport = {
		.codec = A2DP_CODEC_SBC,
		.a2dp = {
			.cconfig = (uint8_t *)&config_sbc_44100_stereo,
			.cconfig_size = sizeof(config_sbc_44100_stereo),
		},
	};

	transport.mtu_write = 153 * 3,
	test_a2dp_encoding(&transport, io_thread_a2dp_source_sbc);

	transport.mtu_read = transport.mtu_write;
	test_a2dp_decoding(&transport, io_thread_a2dp_sink_sbc);

} END_TEST

#if ENABLE_AAC
START_TEST(test_a2dp_aac) {

	struct ba_transport transport = {
		.codec = A2DP_CODEC_MPEG24,
		.a2dp = {
			.cconfig = (uint8_t *)&config_aac_44100_stereo,
			.cconfig_size = sizeof(config_aac_44100_stereo),
		},
	};

	transport.mtu_write = 64;
	test_a2dp_encoding(&transport, io_thread_a2dp_source_aac);

	transport.mtu_read = transport.mtu_write;
	test_a2dp_decoding(&transport, io_thread_a2dp_sink_aac);

} END_TEST
#endif

#if ENABLE_APTX
START_TEST(test_a2dp_aptx) {

	struct ba_transport transport = {
		.codec = A2DP_CODEC_VENDOR_APTX,
		.a2dp = {
			.cconfig = (uint8_t *)&config_aptx_44100_stereo,
			.cconfig_size = sizeof(config_aptx_44100_stereo),
		},
	};

	transport.mtu_write = 40;
	test_a2dp_encoding(&transport, io_thread_a2dp_source_aptx);

} END_TEST
#endif

#if ENABLE_LDAC
START_TEST(test_a2dp_ldac) {

	struct ba_transport transport = {
		.profile = BLUETOOTH_PROFILE_A2DP_SOURCE,
		.codec = A2DP_CODEC_VENDOR_LDAC,
		.a2dp = {
			.cconfig = (uint8_t *)&config_ldac_44100_stereo,
			.cconfig_size = sizeof(config_ldac_44100_stereo),
		},
	};

	transport.mtu_write = RTP_HEADER_LEN + sizeof(rtp_media_header_t) + 679;
	test_a2dp_encoding(&transport, io_thread_a2dp_source_ldac);

} END_TEST
#endif

int main(void) {

	Suite *s = suite_create(__FILE__);
	TCase *tc = tcase_create(__FILE__);
	SRunner *sr = srunner_create(s);

	suite_add_tcase(s, tc);

	tcase_add_test(tc, test_a2dp_sbc);
#if ENABLE_AAC
	config.aac_afterburner = true;
	tcase_add_test(tc, test_a2dp_aac);
#endif
#if ENABLE_APTX
	tcase_add_test(tc, test_a2dp_aptx);
#endif
#if ENABLE_LDAC
	config.ldac_abr = true;
	config.ldac_eqmid = LDACBT_EQMID_HQ;
	tcase_add_test(tc, test_a2dp_ldac);
#endif

	srunner_run_all(sr, CK_ENV);
	int nf = srunner_ntests_failed(sr);
	srunner_free(sr);

	return nf == 0 ? 0 : 1;
}
