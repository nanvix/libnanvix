/*
 * MIT License
 *
 * Copyright(c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis     <davidsondfgl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <nanvix/sys/portal.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/task.h>
#include <nanvix/runtime/fence.h>
#include <posix/errno.h>

#include "test.h"

#if __TARGET_HAS_PORTAL

/*============================================================================*
 * Constants                                                                  *
 *============================================================================*/

/**
 * @name Multiplexation
 */
/**{*/
#define TEST_MULTIPLEXATION_PORTAL_PAIRS  KPORTAL_PORT_NR
#define TEST_MULTIPLEXATION2_PORTAL_PAIRS 3

/* @note Define an even number to this constant. */
#define TEST_MULTIPLEXATION3_PORTAL_MSGS_NR   6
#define TEST_MULTIPLEXATION3_PORTAL_SELECT_NR (TEST_MULTIPLEXATION3_PORTAL_MSGS_NR / 2)

/* @note Define an even number to this constant. */
#define TEST_MLTPX4_PORTAL_SEND_NR    4
#define TEST_MLTPX4_PORTAL_SELECT_NR  (TEST_MLTPX4_PORTAL_SEND_NR / 2)
#define TEST_MLTPX4_PORTAL_SELECT_MSG (TEST_MLTPX4_PORTAL_SEND_NR / TEST_MLTPX4_PORTAL_SELECT_NR)
/**}*/

/**
 * @brief Virtualization
 */
#define TEST_VIRTUALIZATION_PORTALS_NR   KPORTAL_PORT_NR

/**
 * @brief Pending Messages - Unlink
 */
#define TEST_PENDING_UNLINK_PORTAL_PAIRS 2

/**
 * @name Threads
 */
/**{*/
#define TEST_THREAD_MAX          (CORES_NUM - 1)
#define TEST_THREAD_PORTALID_NUM ((TEST_THREAD_NPORTS / TEST_THREAD_MAX) + 1)
#define TEST_CORE_AFFINITY       (1)
#define TEST_THREAD_AFFINITY     (1 << TEST_CORE_AFFINITY)
/**}*/

/*============================================================================*
 * Global variables                                                           *
 *============================================================================*/

PRIVATE char message[PORTAL_SIZE_LARGE];
PRIVATE char message_in[TEST_THREAD_MAX][PORTAL_SIZE];
PRIVATE char message_out[PORTAL_SIZE];

/**
 * @brief Simple fence used for thread synchronization.
 */
PRIVATE struct nanvix_fence _fence;

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Portal Create Unlink
 */
static void test_api_portal_create_unlink(void)
{
	int local;
	int remote;
	int portalid;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_create(local, 0)) >= 0);
	test_assert(kportal_unlink(portalid) == 0);

	test_assert((portalid = kportal_create(local, 0)) >= 0);
	test_assert(kportal_allow(portalid, remote, 0) >= 0);
	test_assert(kportal_unlink(portalid) == 0);
}

/*============================================================================*
 * API Test: Open Close                                                       *
 *============================================================================*/

/**
 * @brief API Test: Portal Open Close
 */
static void test_api_portal_open_close(void)
{
	int local;
	int remote;
	int portalid;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_open(local, remote, 0)) >= 0);
	test_assert(kportal_close(portalid) == 0);
}

/*============================================================================*
 * API Test: Get volume                                                       *
 *============================================================================*/

/**
 * @brief API Test: Portal Get volume
 */
static void test_api_portal_get_volume(void)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	size_t volume;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portal_in = kportal_create(local, 0)) >= 0);
	test_assert((portal_out = kportal_open(local, remote, 0)) >= 0);

		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
		test_assert(volume == 0);

		test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
		test_assert(volume == 0);

	test_assert(kportal_close(portal_out) == 0);
	test_assert(kportal_unlink(portal_in) == 0);
}

/*============================================================================*
 * API Test: Get latency                                                      *
 *============================================================================*/

/**
 * @brief API Test: Portal Get latency
 */
static void test_api_portal_get_latency(void)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	uint64_t latency;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portal_in = kportal_create(local, 0)) >= 0);
	test_assert((portal_out = kportal_open(local, remote, 0)) >= 0);

		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
		test_assert(latency == 0);

		test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
		test_assert(latency == 0);

	test_assert(kportal_close(portal_out) == 0);
	test_assert(kportal_unlink(portal_in) == 0);
}

/*============================================================================*
 * API Test: Get counters                                                     *
 *============================================================================*/

/**
 * @brief API Test: Portal Get latency
 */
static void test_api_portal_get_counters(void)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	uint64_t c0;
	uint64_t c1;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portal_in = kportal_create(local, 0)) >= 0);

		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NCREATES, &c0) == 0);
		test_assert(c0 == 5);
		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NOPENS, &c1) == 0);
		test_assert(c1 == 3);
		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NCLOSES, &c1) == 0);
		test_assert(c1 == 3);

	test_assert((portal_out = kportal_open(local, remote, 0)) >= 0);

		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NCREATES, &c0) == 0);
		test_assert(c0 == 5);
		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NOPENS, &c0) == 0);
		test_assert(c0 == 4);
		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NUNLINKS, &c1) == 0);
		test_assert(c1 == 4);
		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NCLOSES, &c1) == 0);
		test_assert(c1 == 3);

		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NREADS, &c1) == 0);
		test_assert(c1 == 0);
		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NWRITES, &c1) == 0);
		test_assert(c1 == 0);

	test_assert(kportal_close(portal_out) == 0);

		test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NCLOSES, &c1) == 0);
		test_assert(c1 == 4);

	test_assert(kportal_unlink(portal_in) == 0);
}

/*============================================================================*
 * API Test: Read Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read Write
 */
static void test_api_portal_read_write(void)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	size_t volume;
	uint64_t latency;
	uint64_t counter;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portal_in = kportal_create(local, 0)) >= 0);
	test_assert((portal_out = kportal_open(local, remote, 0)) >= 0);

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
	test_assert(latency == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
	test_assert(latency == 0);

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NREADS, &counter) == 0);
	test_assert(counter == 0);
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NWRITES, &counter) == 0);
	test_assert(counter == 0);

	if (local == MASTER_NODENUM)
	{
		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kmemset(message, 0, PORTAL_SIZE);

			test_assert(kportal_allow(portal_in, remote, 0) == 0);
			test_assert(kportal_read(portal_in, message, PORTAL_SIZE) == PORTAL_SIZE);

			for (unsigned j = 0; j < PORTAL_SIZE; ++j)
				test_assert(message[j] == 1);

			kmemset(message, 2, PORTAL_SIZE);

			test_assert(kportal_write(portal_out, message, PORTAL_SIZE) == PORTAL_SIZE);
		}
	}
	else
	{
		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kmemset(message, 1, PORTAL_SIZE);

			test_assert(kportal_write(portal_out, message, PORTAL_SIZE) == PORTAL_SIZE);

			kmemset(message, 0, PORTAL_SIZE);

			test_assert(kportal_allow(portal_in, remote, 0) == 0);
			test_assert(kportal_read(portal_in, message, PORTAL_SIZE) == PORTAL_SIZE);

			for (unsigned j = 0; j < PORTAL_SIZE; ++j)
				test_assert(message[j] == 2);
		}
	}

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE));
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE));
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NREADS, &counter) == 0);
	test_assert(counter == NITERATIONS);
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_NWRITES, &counter) == 0);
	test_assert(counter == NITERATIONS);

	test_assert(kportal_close(portal_out) == 0);
	test_assert(kportal_unlink(portal_in) == 0);
}

/*============================================================================*
 * API Test: Read Write Large                                                 *
 *============================================================================*/

/**
 * @brief API Test: Read Write Large
 */
static void test_api_portal_read_write_large(void)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portal_in = kportal_create(local, 0)) >= 0);
	test_assert((portal_out = kportal_open(local, remote, 0)) >= 0);

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
	test_assert(latency == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
	test_assert(latency == 0);

	if (local == MASTER_NODENUM)
	{
		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kmemset(message, 0, PORTAL_SIZE_LARGE);

			test_assert(kportal_allow(portal_in, remote, 0) == 0);
			test_assert(kportal_read(portal_in, message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);

			for (unsigned j = 0; j < PORTAL_SIZE_LARGE; ++j)
				test_assert(message[j] == 1);

			kmemset(message, 2, PORTAL_SIZE_LARGE);

			test_assert(kportal_write(portal_out, message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);
		}
	}
	else
	{
		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kmemset(message, 1, PORTAL_SIZE_LARGE);

			test_assert(kportal_write(portal_out, message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);

			kmemset(message, 0, PORTAL_SIZE_LARGE);

			test_assert(kportal_allow(portal_in, remote, 0) == 0);
			test_assert(kportal_read(portal_in, message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);

			for (unsigned j = 0; j < PORTAL_SIZE_LARGE; ++j)
				test_assert(message[j] == 2);
		}
	}

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE_LARGE));
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE_LARGE));
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_close(portal_out) == 0);
	test_assert(kportal_unlink(portal_in) == 0);
}

/*============================================================================*
 * API Test: Virtualization                                                   *
 *============================================================================*/

/**
 * @brief API Test: Virtualization of HW portals.
 */
static void test_api_portal_virtualization(void)
{
	int local;
	int remote;
	int portal_in[TEST_VIRTUALIZATION_PORTALS_NR];
	int portal_out[TEST_VIRTUALIZATION_PORTALS_NR];

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	/* Creates multiple virtual portals. */
	for (unsigned i = 0; i < TEST_VIRTUALIZATION_PORTALS_NR; ++i)
	{
		test_assert((portal_in[i] = kportal_create(local, i)) >= 0);
		test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);
	}

	/* Deletion of the created virtual portals. */
	for (unsigned i = 0; i < TEST_VIRTUALIZATION_PORTALS_NR; ++i)
	{
		test_assert(kportal_unlink(portal_in[i]) == 0);
		test_assert(kportal_close(portal_out[i]) == 0);
	}
}

/*============================================================================*
 * API Test: Multiplexation                                                   *
 *============================================================================*/

/**
 * @brief API Test: Multiplexation of virtual to hardware portals.
 */
static void test_api_portal_multiplexation(void)
{
	int local;
	int remote;
	int portal_in[TEST_MULTIPLEXATION_PORTAL_PAIRS];
	int portal_out[TEST_MULTIPLEXATION_PORTAL_PAIRS];
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	/* Creates multiple virtual portals. */
	for (unsigned i = 0; i < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++i)
	{
		test_assert((portal_in[i] = kportal_create(local, i)) >= 0);
		test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);
	}

	/* Multiple write/read operations to test multiplexation. */
	if (local == MASTER_NODENUM)
	{
		for (unsigned i = 0; i < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++i)
		{
			kmemset(message, (i - 1), PORTAL_SIZE);

			test_assert(kportal_allow(portal_in[i], remote, i) == 0);
			test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE) == PORTAL_SIZE);

			for (unsigned j = 0; j < PORTAL_SIZE; ++j)
				test_assert((message[j] - i) == 0);

			kmemset(message, (i + 1), PORTAL_SIZE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE) == PORTAL_SIZE);
		}
	}
	else
	{
		for (unsigned i = 0; i < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++i)
		{
			kmemset(message, i, PORTAL_SIZE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE) == PORTAL_SIZE);

			kmemset(message, i, PORTAL_SIZE);

			test_assert(kportal_allow(portal_in[i], remote, i) == 0);
			test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE) == PORTAL_SIZE);

			for (unsigned j = 0; j < PORTAL_SIZE; ++j)
				test_assert((message[j] - i - 1) == 0);
		}
	}

	/* Checks the data volume transferred by each vportal. */
	for (unsigned i = 0; i < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++i)
	{
		test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
		test_assert(volume == PORTAL_SIZE);
		test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

		test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
		test_assert(volume == PORTAL_SIZE);
		test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

		/* Deletion of the created virtual portals. */
		test_assert(kportal_unlink(portal_in[i]) == 0);
		test_assert(kportal_close(portal_out[i]) == 0);
	}
}

/*============================================================================*
 * API Test: Allow                                                            *
 *============================================================================*/

/**
 * @brief API Test: Virtual portals allowing.
 */
static void test_api_portal_allow(void)
{
	int local;
	int remote;
	int portal_in[2];
	int portal_out[2];

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	if (local == SLAVE_NODENUM)
	{
		for (unsigned i = 0; i < 2; ++i)
			test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);

		for (unsigned i = 0; i < 2; ++i)
		{
			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE) == PORTAL_SIZE);

			test_assert(kportal_close(portal_out[i]) == 0);
		}
	}
	else
	{
		/* Creates the input vportals. */
		for (unsigned i = 0; i < 2; ++i)
			test_assert((portal_in[i] = kportal_create(local, i)) >= 0);

		/* Allowing tests. */
		test_assert(kportal_allow(portal_in[0], remote, 0) == 0);
		test_assert(kportal_read(portal_in[1], message, PORTAL_SIZE) == -EACCES);
		test_assert(kportal_read(portal_in[0], message, PORTAL_SIZE) == PORTAL_SIZE);
		test_assert(kportal_read(portal_in[0], message, PORTAL_SIZE) == -EACCES);
		test_assert(kportal_allow(portal_in[1], remote, 1) == 0);
		test_assert(kportal_read(portal_in[0], message, PORTAL_SIZE) == -EACCES);
		test_assert(kportal_read(portal_in[1], message, PORTAL_SIZE) == PORTAL_SIZE);

		/* Unlinks the created vportals. */
		for (unsigned i = 0; i < 2; ++i)
			test_assert(kportal_unlink(portal_in[i]) == 0);
	}
}

/*============================================================================*
 * API Test: Multiplexation - Out of Order                                    *
 *============================================================================*/

/**
 * @brief API Test: Multiplexation test to assert the correct working when
 * messages flow occur in diferent order than the expected.
 */
static void test_api_portal_multiplexation_2(void)
{
	int local;
	int remote;
	int portal_in[TEST_MULTIPLEXATION2_PORTAL_PAIRS];
	int portal_out[TEST_MULTIPLEXATION2_PORTAL_PAIRS];
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	/* Creates multiple virtual portals. */
	for (unsigned i = 0; i < TEST_MULTIPLEXATION2_PORTAL_PAIRS; ++i)
	{
		test_assert((portal_in[i] = kportal_create(local, i)) >= 0);
		test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);
	}

	if (local == MASTER_NODENUM)
	{
		/* Read the messages in descendant order. */
		for (int i = (TEST_MULTIPLEXATION2_PORTAL_PAIRS - 1); i >= 0; --i)
		{
			kmemset(message, i, PORTAL_SIZE);

			test_assert(kportal_allow(portal_in[i], remote, i) == 0);
			test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE) == PORTAL_SIZE);

			for (unsigned j = 0; j < PORTAL_SIZE; ++j)
				test_assert((message[j] - i) == 0);

			/* Checks the data transfered by each vportal. */
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE);
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
		}
	}
	else
	{
		for (unsigned i = 0; i < TEST_MULTIPLEXATION2_PORTAL_PAIRS; ++i)
		{
			kmemset(message, i, PORTAL_SIZE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE) == PORTAL_SIZE);

			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE);
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
		}
	}

	/* Deletion of the created virtual portals. */
	for (unsigned i = 0; i < TEST_MULTIPLEXATION2_PORTAL_PAIRS; ++i)
	{
		test_assert(kportal_unlink(portal_in[i]) == 0);
		test_assert(kportal_close(portal_out[i]) == 0);
	}
}

/*============================================================================*
 * API Test: Multiplexation - Messages Selection                              *
 *============================================================================*/

/**
 * @brief API Test: Multiplexation test to assert the correct message selection
 * of multiple messages.
 *
 * @details In this test, the MASTER node sends 4 messages but only 2 for open ports
 * on the SLAVE which need to select them correctly.
 */
static void test_api_portal_multiplexation_3(void)
{
	int local;
	int remote;
	int portal_in[TEST_MULTIPLEXATION3_PORTAL_SELECT_NR];
	int portal_out[TEST_MULTIPLEXATION3_PORTAL_MSGS_NR];
	int port;
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	if (local == SLAVE_NODENUM)
	{
		/* Opens the output vportals. */
		for (unsigned int i = 0; i < TEST_MULTIPLEXATION3_PORTAL_MSGS_NR; ++i)
			test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);

		/* Sends all the messages to the SLAVE node. */
		for (int i = (TEST_MULTIPLEXATION3_PORTAL_MSGS_NR - 1); i >= 0; --i)
		{
			kmemset(message, i, PORTAL_SIZE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE) == PORTAL_SIZE);
		}

		/* Closes the opened vportals. */
		for (unsigned int i = 0; i < TEST_MULTIPLEXATION3_PORTAL_MSGS_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE);

			test_assert(kportal_close(portal_out[i]) == 0);
		}
	}
	else
	{
		/* Creates multiple virtual portals (half of the MASTER senders). */
		for (int i = 0; i < TEST_MULTIPLEXATION3_PORTAL_SELECT_NR; ++i)
			test_assert((portal_in[i] = kportal_create(local, (i*2))) >= 0);

		for (unsigned i = 0; i < TEST_MULTIPLEXATION3_PORTAL_SELECT_NR; ++i)
		{
			port = kcomm_get_port(portal_in[i], COMM_TYPE_PORTAL);

			kmemset(message, -1, PORTAL_SIZE);

			test_assert(kportal_allow(portal_in[i], remote, port) == 0);
			test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE) == PORTAL_SIZE);

			for (unsigned j = 0; j < PORTAL_SIZE; ++j)
				test_assert((message[j] - port) == 0);
		}

		/* Checks the data volume transferred by each vportal. */
		for (unsigned i = 0; i < TEST_MULTIPLEXATION3_PORTAL_SELECT_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE);
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

			/* Unlinks each vportal. */
			test_assert(kportal_unlink(portal_in[i]) == 0);
		}
	}
}

/*============================================================================*
 * API Test: Multiplexation - Multiple Messages                               *
 *============================================================================*/

/**
 * @brief API Test: Multiplexation test to assert the message buffers tab
 * solution to support multiple messages on the same port simultaneously.
 *
 * @details In this test, the SLAVE sends 4 messages to 2 ports of the SLAVE.
 * The reads in the master are also made out of order to assure that there are
 * 2 messages queued for each vportal on MASTER.
 */
static void test_api_portal_multiplexation_4(void)
{
	int local;
	int remote;
	int portal_in[TEST_MLTPX4_PORTAL_SELECT_NR];
	int portal_out[TEST_MLTPX4_PORTAL_SEND_NR];
	int port;
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	if (local == MASTER_NODENUM)
	{
		/* Creates multiple virtual portals. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SELECT_NR; ++i)
			test_assert((portal_in[i] = kportal_create(local, i)) >= 0);

		/* For each input vportal. */
		for (int i = (TEST_MLTPX4_PORTAL_SELECT_NR - 1); i >= 0; --i)
		{
			/* For each message it should receive. */
			for (int j = (TEST_MLTPX4_PORTAL_SELECT_MSG - 1); j >= 0; --j)
			{
				port = (i*2 + j);

				kmemset(message, -1, PORTAL_SIZE);

				test_assert(kportal_allow(portal_in[i], remote, port) == 0);
				test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE) == PORTAL_SIZE);

				/* Asserts the message data. */
				for (unsigned k = 0; k < PORTAL_SIZE; ++k)
					test_assert((message[k] - port) == 0);
			}
		}

		/* Checks the data volume transferred by each vportal. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SELECT_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == (PORTAL_SIZE * TEST_MLTPX4_PORTAL_SELECT_MSG));
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

			/* Unlinks the created vportals. */
			test_assert(kportal_unlink(portal_in[i]) == 0);
		}
	}
	else
	{
		/* Opens the output vportals. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SEND_NR; ++i)
			test_assert((portal_out[i] = kportal_open(local, remote, (int) (i/TEST_MLTPX4_PORTAL_SELECT_MSG))) >= 0);

		/* Sends all the messages to the MASTER node. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SEND_NR; ++i)
		{
			kmemset(message, i, PORTAL_SIZE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE) == PORTAL_SIZE);
		}

		/* Checks the data volume transferred by each vportal. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SEND_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE);

			/* Closes the output vportals. */
			test_assert(kportal_close(portal_out[i]) == 0);
		}
	}
}

/**
 * @brief API Test: Multiplexation test to assert the correct working when
 * messages flow occur in diferent order than the expected.
 */
static void test_api_portal_multiplexation_2_large(void)
{
	int local;
	int remote;
	int portal_in[TEST_MULTIPLEXATION2_PORTAL_PAIRS];
	int portal_out[TEST_MULTIPLEXATION2_PORTAL_PAIRS];
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	/* Creates multiple virtual portals. */
	for (unsigned i = 0; i < TEST_MULTIPLEXATION2_PORTAL_PAIRS; ++i)
	{
		test_assert((portal_in[i] = kportal_create(local, i)) >= 0);
		test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);
	}

	if (local == MASTER_NODENUM)
	{
		/* Read the messages in descendant order. */
		for (int i = (TEST_MULTIPLEXATION2_PORTAL_PAIRS - 1); i >= 0; --i)
		{
			kmemset(message, i, PORTAL_SIZE_LARGE);

			test_assert(kportal_allow(portal_in[i], remote, i) == 0);
			test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);

			for (unsigned j = 0; j < PORTAL_SIZE_LARGE; ++j)
				test_assert((message[j] - i) == 0);

			/* Checks the data transfered by each vportal. */
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE_LARGE);
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
		}
	}
	else
	{
		for (unsigned i = 0; i < TEST_MULTIPLEXATION2_PORTAL_PAIRS; ++i)
		{
			kmemset(message, i, PORTAL_SIZE_LARGE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);

			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE_LARGE);
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
		}
	}

	/* Deletion of the created virtual portals. */
	for (unsigned i = 0; i < TEST_MULTIPLEXATION2_PORTAL_PAIRS; ++i)
	{
		test_assert(kportal_unlink(portal_in[i]) == 0);
		test_assert(kportal_close(portal_out[i]) == 0);
	}
}

/*============================================================================*
 * API Test: Multiplexation - Messages Selection                              *
 *============================================================================*/

/**
 * @brief API Test: Multiplexation test to assert the correct message selection
 * of multiple messages.
 *
 * @details In this test, the MASTER node sends 4 messages but only 2 for open ports
 * on the SLAVE which need to select them correctly.
 */
static void test_api_portal_multiplexation_3_large(void)
{
	int local;
	int remote;
	int portal_in[TEST_MULTIPLEXATION3_PORTAL_SELECT_NR];
	int portal_out[TEST_MULTIPLEXATION3_PORTAL_MSGS_NR];
	int port;
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	if (local == SLAVE_NODENUM)
	{
		/* Opens the output vportals. */
		for (unsigned int i = 0; i < TEST_MULTIPLEXATION3_PORTAL_MSGS_NR; ++i)
			test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);

		/* Sends all the messages to the SLAVE node. */
		for (int i = (TEST_MULTIPLEXATION3_PORTAL_MSGS_NR - 1); i >= 0; --i)
		{
			kmemset(message, i, PORTAL_SIZE_LARGE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);
		}

		/* Closes the opened vportals. */
		for (unsigned int i = 0; i < TEST_MULTIPLEXATION3_PORTAL_MSGS_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE_LARGE);

			test_assert(kportal_close(portal_out[i]) == 0);
		}
	}
	else
	{
		/* Creates multiple virtual portals (half of the MASTER senders). */
		for (int i = 0; i < TEST_MULTIPLEXATION3_PORTAL_SELECT_NR; ++i)
			test_assert((portal_in[i] = kportal_create(local, (i*2))) >= 0);

		for (unsigned i = 0; i < TEST_MULTIPLEXATION3_PORTAL_SELECT_NR; ++i)
		{
			port = kcomm_get_port(portal_in[i], COMM_TYPE_PORTAL);

			kmemset(message, -1, PORTAL_SIZE_LARGE);

			test_assert(kportal_allow(portal_in[i], remote, port) == 0);
			test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);

			for (unsigned j = 0; j < PORTAL_SIZE_LARGE; ++j)
				test_assert((message[j] - port) == 0);
		}

		/* Checks the data volume transferred by each vportal. */
		for (unsigned i = 0; i < TEST_MULTIPLEXATION3_PORTAL_SELECT_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE_LARGE);
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

			/* Unlinks each vportal. */
			test_assert(kportal_unlink(portal_in[i]) == 0);
		}
	}
}

/*============================================================================*
 * API Test: Multiplexation - Multiple Messages                               *
 *============================================================================*/

/**
 * @brief API Test: Multiplexation test to assert the message buffers tab
 * solution to support multiple messages on the same port simultaneously.
 *
 * @details In this test, the SLAVE sends 4 messages to 2 ports of the SLAVE.
 * The reads in the master are also made out of order to assure that there are
 * 2 messages queued for each vportal on MASTER.
 */
static void test_api_portal_multiplexation_4_large(void)
{
	int local;
	int remote;
	int portal_in[TEST_MLTPX4_PORTAL_SELECT_NR];
	int portal_out[TEST_MLTPX4_PORTAL_SEND_NR];
	int port;
	size_t volume;
	uint64_t latency;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	if (local == MASTER_NODENUM)
	{
		/* Creates multiple virtual portals. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SELECT_NR; ++i)
			test_assert((portal_in[i] = kportal_create(local, i)) >= 0);

		/* For each input vportal. */
		for (int i = (TEST_MLTPX4_PORTAL_SELECT_NR - 1); i >= 0; --i)
		{
			/* For each message it should receive. */
			for (int j = (TEST_MLTPX4_PORTAL_SELECT_MSG - 1); j >= 0; --j)
			{
				port = (i*2 + j);

				kmemset(message, -1, PORTAL_SIZE_LARGE);

				test_assert(kportal_allow(portal_in[i], remote, port) == 0);
				test_assert(kportal_read(portal_in[i], message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);

				/* Asserts the message data. */
				for (unsigned k = 0; k < PORTAL_SIZE_LARGE; ++k)
					test_assert((message[k] - port) == 0);
			}
		}

		/* Checks the data volume transferred by each vportal. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SELECT_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == (PORTAL_SIZE_LARGE * TEST_MLTPX4_PORTAL_SELECT_MSG));
			test_assert(kportal_ioctl(portal_in[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

			/* Unlinks the created vportals. */
			test_assert(kportal_unlink(portal_in[i]) == 0);
		}
	}
	else
	{
		/* Opens the output vportals. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SEND_NR; ++i)
			test_assert((portal_out[i] = kportal_open(local, remote, (int) (i/TEST_MLTPX4_PORTAL_SELECT_MSG))) >= 0);

		/* Sends all the messages to the MASTER node. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SEND_NR; ++i)
		{
			kmemset(message, i, PORTAL_SIZE_LARGE);

			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE_LARGE) == PORTAL_SIZE_LARGE);
		}

		/* Checks the data volume transferred by each vportal. */
		for (unsigned i = 0; i < TEST_MLTPX4_PORTAL_SEND_NR; ++i)
		{
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE_LARGE);

			/* Closes the output vportals. */
			test_assert(kportal_close(portal_out[i]) == 0);
		}
	}
}

/*============================================================================*
 * API Test: Pending Messages - Unlink                                        *
 *============================================================================*/

/**
 * @brief API Test: Test to assert if the kernel correctly avoids an unlink
 * call when there still messages addressed to the target vportal on message
 * buffers tab.
 *
 * @todo Uncomment kportal_wait() call when microkernel properly supports it.
 */
static void test_api_portal_pending_msg_unlink(void)
{
	int local;
	int remote;
	size_t volume;
	uint64_t latency;
	int portal_in[TEST_PENDING_UNLINK_PORTAL_PAIRS];
	int portal_out[TEST_PENDING_UNLINK_PORTAL_PAIRS];

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	if (local == SLAVE_NODENUM)
	{
		/* Opens the output portals. */
		for (unsigned i = 0; i < TEST_PENDING_UNLINK_PORTAL_PAIRS; ++i)
			test_assert((portal_out[i] = kportal_open(local, remote, i)) >= 0);

		for (int i = 0; i < TEST_PENDING_UNLINK_PORTAL_PAIRS; ++i)
			test_assert(kportal_write(portal_out[i], message, PORTAL_SIZE) == PORTAL_SIZE);

		/* Checks the data volume transfered by each vportal. */
		for (unsigned i = 0; i < TEST_PENDING_UNLINK_PORTAL_PAIRS; ++i)
		{
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
			test_assert(volume == PORTAL_SIZE);
			test_assert(kportal_ioctl(portal_out[i], KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

			/* Closes the opened vportals. */
			test_assert(kportal_close(portal_out[i]) == 0);
		}
	}
	else
	{
		/* Creates the input portals. */
		for (unsigned i = 0; i < TEST_PENDING_UNLINK_PORTAL_PAIRS; ++i)
			test_assert((portal_in[i] = kportal_create(local, i)) >= 0);

		/* Unlink tests. */
		test_assert(kportal_allow(portal_in[1], remote, 1) == 0);
		test_assert(kportal_read(portal_in[1], message, PORTAL_SIZE) == PORTAL_SIZE);

		test_assert(kportal_ioctl(portal_in[1], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
		test_assert(volume == PORTAL_SIZE);

		test_assert(kportal_unlink(portal_in[1]) == 0);


		test_assert(kportal_unlink(portal_in[0]) == -EBUSY);

		test_assert(kportal_allow(portal_in[0], remote, 0) == 0);
		test_assert(kportal_read(portal_in[0], message, PORTAL_SIZE) == PORTAL_SIZE);

		test_assert(kportal_ioctl(portal_in[0], KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
		test_assert(volume == PORTAL_SIZE);

		test_assert(kportal_unlink(portal_in[0]) == 0);
	}
}

/*============================================================================*
 * API Test: Message Forwarding                                               *
 *============================================================================*/

/**
 * @brief API Test: Message forwarding
 */
static void test_api_portal_msg_forwarding(void)
{
	int local;
	int portal_in;
	int portal_out;
	size_t volume;
	uint64_t latency;

	local = knode_get_num();

	test_assert((portal_in = kportal_create(local, 0)) >= 0);
	test_assert((portal_out = kportal_open(local, local, 0)) >= 0);

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
	test_assert(latency == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
	test_assert(latency == 0);

	for (unsigned i = 0; i < NITERATIONS; i++)
	{
		test_assert(kportal_allow(portal_in, local, 0) == 0);

		kmemset(message, local, PORTAL_SIZE);

		test_assert(kportal_write(portal_out, message, PORTAL_SIZE) == PORTAL_SIZE);

		kmemset(message, -1, PORTAL_SIZE);

		test_assert(kportal_read(portal_in, message, PORTAL_SIZE) == PORTAL_SIZE);

		for (unsigned j = 0; j < PORTAL_SIZE; ++j)
			test_assert(message[j] == local);
	}

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE));
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE));
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_close(portal_out) == 0);
	test_assert(kportal_unlink(portal_in) == 0);
}

#if __NANVIX_USE_COMM_WITH_TASKS

/*============================================================================*
 * API Test: Mailbox on the Dispatcher                                        *
 *============================================================================*/

#define WT(x)      ((word_t) x)
#define TEST_DHARD KTASK_DEPENDENCY_HARD

PRIVATE int dispatcher_init(
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4
)
{
	int portal_in;
	int portal_out;
	size_t volume;
	uint64_t latency;

	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(arg4);

	portal_in  = (int) arg0;
	portal_out = (int) arg1;

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == 0);
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	return (0);
}

PRIVATE int dispatcher_set_message(
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4
)
{
	char value;
	char * msg;

	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(arg4);

	msg   = (char *) arg0;
	value = (char) arg1;

	kmemset(msg, value, PORTAL_SIZE);

	return (0);
}

PRIVATE int dispatcher_reset_message(
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4
)
{
	char * msg;

	UNUSED(arg1);
	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(arg4);

	msg = (char *) arg0;

	kmemset(msg, 0, PORTAL_SIZE);

	return (0);
}

PRIVATE int dispatcher_check_message(
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4
)
{
	char value;
	char * msg;

	UNUSED(arg0);
	UNUSED(arg1);
	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(arg4);

	msg   = (char *) arg0;
	value = (char) arg1;

	for (unsigned i = 0; i < PORTAL_SIZE; ++i)
		test_assert(msg[i] == value);

	return (0);
}

PRIVATE int dispatcher_loop(
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4
)
{
	int * iteration;

	UNUSED(arg1);
	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(arg4);

	iteration = (int *) arg0;

	(*iteration)++;

	/* Continue the loop? */
	if (*iteration < NITERATIONS)
		ktask_exit0(0, KTASK_MANAGEMENT_USER1);

	/* Finish the loop. */
	else
		ktask_exit0(0, KTASK_MANAGEMENT_USER0);

	return (0);
}

PRIVATE int dispatcher_finish(
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4
)
{
	int portal_in;
	int portal_out;
	size_t volume;
	uint64_t latency;

	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(arg4);

	portal_in  = (int) arg0;
	portal_out = (int) arg1;

	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE));
	test_assert(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);
	test_assert(volume == (NITERATIONS * PORTAL_SIZE));
	test_assert(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);

	return (0);
}

/**
 * @brief API Test: Mailbox on the Dispatcher
 */
static void test_api_portal_on_dispatcher(void)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	int setvalue;
	int checkvalue;
	int iteration;

	ktask_t init, set, reset, check, loop, finish;
	ktask_t awrite, aread, allow, wwait, rwait;

	local     = knode_get_num();
	remote    = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;
	iteration = 0;

	test_assert((portal_in = kportal_create(local, 0)) >= 0);
	test_assert((portal_out = kportal_open(local, remote, 0)) >= 0);

	test_assert(ktask_create(&init,   dispatcher_init,          KTASK_PRIORITY_LOW, 0, KTASK_MANAGEMENT_DEFAULT) == 0);
	test_assert(ktask_create(&set,    dispatcher_set_message,   KTASK_PRIORITY_LOW, 0, KTASK_MANAGEMENT_DEFAULT) == 0);
	test_assert(ktask_create(&reset,  dispatcher_reset_message, KTASK_PRIORITY_LOW, 0, KTASK_MANAGEMENT_DEFAULT) == 0);
	test_assert(ktask_create(&check,  dispatcher_check_message, KTASK_PRIORITY_LOW, 0, KTASK_MANAGEMENT_DEFAULT) == 0);
	test_assert(ktask_create(&loop,   dispatcher_loop,          KTASK_PRIORITY_LOW, 0, KTASK_MANAGEMENT_DEFAULT) == 0);
	test_assert(ktask_create(&finish, dispatcher_finish,        KTASK_PRIORITY_LOW, 0, KTASK_MANAGEMENT_DEFAULT) == 0);

	test_assert(ktask_portal_write(&awrite, &wwait) == 0);
	test_assert(ktask_portal_read(&allow, &aread, &rwait) == 0);

	if (local == MASTER_NODENUM)
	{
		test_assert(ktask_connect(&init,  &set,    false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&set,   &awrite, false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&wwait, &reset,  false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&reset, &allow,  false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&rwait, &check,  false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&check, &loop,   false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&loop,  &set,    false, false, KTASK_TRIGGER_USER1)   == 0);
		test_assert(ktask_connect(&loop,  &finish, false, false, KTASK_TRIGGER_DEFAULT) == 0);

		setvalue   = 1;
		checkvalue = 2;
	}
	else
	{
		test_assert(ktask_connect(&init,  &reset,  false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&reset, &allow,  false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&rwait, &check,  false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&check, &set,    false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&set,   &awrite, false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&wwait, &loop,   false, false, KTASK_TRIGGER_DEFAULT) == 0);
		test_assert(ktask_connect(&loop,  &reset,  false, false, KTASK_TRIGGER_USER1)   == 0);
		test_assert(ktask_connect(&loop,  &finish, false, false, KTASK_TRIGGER_DEFAULT) == 0);

		setvalue   = 2;
		checkvalue = 1;
	}

	ktask_set_arguments(&set, WT(message), WT(setvalue), 0, 0, 0);
	ktask_set_arguments(&reset, WT(message), 0, 0, 0, 0);
	ktask_set_arguments(&check, WT(message), WT(checkvalue), 0, 0, 0);
	ktask_set_arguments(&awrite, WT(portal_out), WT(message), PORTAL_SIZE, 0, 0);
	ktask_set_arguments(&allow, WT(portal_in), WT(message), PORTAL_SIZE, WT(remote), 0);
	ktask_set_arguments(&rwait, WT(portal_in), 0, 0, 0, 0);
	ktask_set_arguments(&loop, WT(&iteration), 0, 0, 0, 0);
	ktask_set_arguments(&finish, WT(portal_in), WT(portal_out), 0, 0, 0);

	test_assert(ktask_dispatch2(&init, WT(portal_in), WT(portal_out)) == 0);
	test_assert(ktask_wait(&finish) == 0);

	if (local == MASTER_NODENUM)
	{
		test_assert(ktask_disconnect(&init,   &set)    == 0);
		test_assert(ktask_disconnect(&set,    &awrite) == 0);
		test_assert(ktask_disconnect(&awrite, &wwait)  == 0);
		test_assert(ktask_disconnect(&wwait,  &reset)  == 0);
		test_assert(ktask_disconnect(&reset,  &allow)  == 0);
		test_assert(ktask_disconnect(&allow,  &aread)  == 0);
		test_assert(ktask_disconnect(&aread,  &rwait)  == 0);
		test_assert(ktask_disconnect(&rwait,  &check)  == 0);
		test_assert(ktask_disconnect(&check,  &loop)   == 0);
		test_assert(ktask_disconnect(&loop,   &set)    == 0);
		test_assert(ktask_disconnect(&loop,   &finish) == 0);
	}
	else
	{
		test_assert(ktask_disconnect(&init,   &reset)  == 0);
		test_assert(ktask_disconnect(&reset,  &allow)  == 0);
		test_assert(ktask_disconnect(&allow,  &aread)  == 0);
		test_assert(ktask_disconnect(&aread,  &rwait)  == 0);
		test_assert(ktask_disconnect(&rwait,  &check)  == 0);
		test_assert(ktask_disconnect(&check,  &set)    == 0);
		test_assert(ktask_disconnect(&set,    &awrite) == 0);
		test_assert(ktask_disconnect(&awrite, &wwait)  == 0);
		test_assert(ktask_disconnect(&wwait,  &loop)   == 0);
		test_assert(ktask_disconnect(&loop,   &reset)  == 0);
		test_assert(ktask_disconnect(&loop,   &finish) == 0);
	}

	test_assert(ktask_unlink(&init) == 0);
	test_assert(ktask_unlink(&set) == 0);
	test_assert(ktask_unlink(&reset) == 0);
	test_assert(ktask_unlink(&check) == 0);
	test_assert(ktask_unlink(&awrite) == 0);
	test_assert(ktask_unlink(&wwait) == 0);
	test_assert(ktask_unlink(&allow) == 0);
	test_assert(ktask_unlink(&aread) == 0);
	test_assert(ktask_unlink(&rwait) == 0);
	test_assert(ktask_unlink(&loop) == 0);
	test_assert(ktask_unlink(&finish) == 0);

	test_assert(kportal_close(portal_out) == 0);
	test_assert(kportal_unlink(portal_in) == 0);
}

#endif

/*============================================================================*
 * Fault Test: Invalid Create                                                 *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Create
 */
static void test_fault_portal_invalid_create(void)
{
	int local;
	int remote;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert(kportal_create(-1, 0) == -EINVAL);
	test_assert(kportal_create(PROCESSOR_NOC_NODES_NUM, 0) == -EINVAL);
	test_assert(kportal_create(remote, 0) == -EINVAL);
	test_assert(kportal_create(local, -1) == -EINVAL);
	test_assert(kportal_create(local, KPORTAL_PORT_NR) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Double Create                                                  *
 *============================================================================*/

/**
 * @brief Fault Test: Double Create
 */
static void test_fault_portal_double_create(void)
{
	int local;
	int portal;

	local = knode_get_num();

	test_assert((portal = kportal_create(local, 0)) >= 0);

	test_assert(kportal_create(local, 0) == -EBUSY);

	test_assert(kportal_unlink(portal) == 0);
}

/*============================================================================*
 * Fault Test: Invalid Unlink                                                 *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Unlink
 */
static void test_fault_portal_invalid_unlink(void)
{
	test_assert(kportal_unlink(-1) == -EINVAL);
	test_assert(kportal_unlink(KPORTAL_MAX) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Double Unlink                                                  *
 *============================================================================*/

/**
 * @brief Fault Test: Double Unlink
 */
static void test_fault_portal_double_unlink(void)
{
	int local;
	int portalid1;
	int portalid2;

	local = knode_get_num();

	test_assert((portalid1 = kportal_create(local, 0)) >= 0);
	test_assert((portalid2 = kportal_create(local, 1)) >= 0);

	test_assert(kportal_unlink(portalid1) == 0);
	test_assert(kportal_unlink(portalid1) == -EBADF);
	test_assert(kportal_unlink(portalid2) == 0);
}

/*============================================================================*
 * Fault Test: Bad Unlink                                                     *
 *============================================================================*/

/**
 * @brief Fault Test: Bad Unlink
 */
static void test_fault_portal_bad_unlink(void)
{
	int local;
	int remote;
	int portalid;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_open(local, remote, 0)) >= 0);
	test_assert(kportal_unlink(portalid) == -EBADF);
	test_assert(kportal_close(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Invalid Open                                                   *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Open
 */
static void test_fault_portal_invalid_open(void)
{
	int local;
	int remote;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert(kportal_open(local, -1, 0) == -EINVAL);
	test_assert(kportal_open(local, PROCESSOR_NOC_NODES_NUM, 0) == -EINVAL);
	test_assert(kportal_open(-1, remote, 0) == -EINVAL);
	test_assert(kportal_open(PROCESSOR_NOC_NODES_NUM, remote, 0) == -EINVAL);
	test_assert(kportal_open(remote, remote, 0) == -EINVAL);
	test_assert(kportal_open(-1, -1, 0) == -EINVAL);
	test_assert(kportal_open(local, remote, -1) == -EINVAL);
	test_assert(kportal_open(local, remote, KPORTAL_PORT_NR) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Invalid Close                                                  *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Close
 */
static void test_fault_portal_invalid_close(void)
{
	test_assert(kportal_close(-1) == -EINVAL);
	test_assert(kportal_close(KPORTAL_MAX) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Double Close                                                   *
 *============================================================================*/

/**
 * @brief Fault Test: Double Close
 */
static void test_fault_portal_double_close(void)
{
	int local;
	int remote;
	int portalid1;
	int portalid2;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid1 = kportal_open(local, remote, 0)) >= 0);
	test_assert((portalid2 = kportal_open(local, remote, 1)) >= 0);

	test_assert(kportal_close(portalid1) == 0);
	test_assert(kportal_close(portalid1) == -EBADF);
	test_assert(kportal_close(portalid2) == 0);
}

/*============================================================================*
 * Fault Test: Bad Close                                                      *
 *============================================================================*/

/**
 * @brief Fault Test: Bad Close
 */
static void test_fault_portal_bad_close(void)
{
	int local;
	int portalid;

	local = knode_get_num();

	test_assert((portalid = kportal_create(local, 0)) >= 0);
	test_assert(kportal_close(portalid) == -EBADF);
	test_assert(kportal_unlink(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Invalid allow                                                  *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid allow
 */
static void test_fault_portal_invalid_allow(void)
{
	int local;
	int remote;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert(kportal_allow(-1, remote, 0) == -EINVAL);
	test_assert(kportal_allow(KPORTAL_MAX, remote, 0) == -EINVAL);
	test_assert(kportal_allow(0, -1, 0) == -EINVAL);
	test_assert(kportal_allow(0, PROCESSOR_NOC_NODES_NUM, 0) == -EINVAL);
	test_assert(kportal_allow(0, remote, -1) == -EINVAL);
	test_assert(kportal_allow(0, remote, KPORTAL_PORT_NR) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Double allow                                                   *
 *============================================================================*/

/**
 * @brief Fault Test: Double allow
 */
static void test_fault_portal_double_allow(void)
{
	int local;
	int remote;
	int portalid;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_create(local, 0)) >= 0);
	test_assert(kportal_allow(portalid, remote, 0) == 0);
	test_assert(kportal_allow(portalid, remote, 0) == -EBUSY);
	test_assert(kportal_unlink(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Bad allow                                                      *
 *============================================================================*/

/**
 * @brief Fault Test: Bad allow
 */
static void test_fault_portal_bad_allow(void)
{
	int local;
	int remote;
	int portalid;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_open(local, remote, 0)) >= 0);
	test_assert(kportal_allow(portalid, remote, 0) == -EBADF);
	test_assert(kportal_close(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Invalid Read                                                   *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Read
 */
static void test_fault_portal_invalid_read(void)
{
	char buffer[PORTAL_SIZE];

	test_assert(kportal_read(-1, buffer, PORTAL_SIZE) == -EINVAL);
	test_assert(kportal_read(KPORTAL_MAX, buffer, PORTAL_SIZE) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Bad Read                                                       *
 *============================================================================*/

/**
 * @brief Fault Test: Bad Read
 */
static void test_fault_portal_bad_read(void)
{
	int local;
	int remote;
	int portalid;
	char buffer[PORTAL_SIZE];

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_open(local, remote, 0)) >= 0);

	test_assert(kportal_read(portalid, buffer, PORTAL_SIZE) == -EBADF);

	test_assert(kportal_close(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Invalid Read Size                                              *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Read Size
 */
static void test_fault_portal_invalid_read_size(void)
{
	int portalid;
	int local;
	char buffer[PORTAL_SIZE];

	local = knode_get_num();

	test_assert((portalid = kportal_create(local, 0)) >= 0);

		test_assert(kportal_read(portalid, buffer, -1) == -EINVAL);
		test_assert(kportal_read(portalid, buffer, 0) == -EINVAL);
		test_assert(kportal_read(portalid, buffer, KPORTAL_MAX_SIZE + 1) == -EINVAL);

	test_assert(kportal_unlink(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Null Read                                                      *
 *============================================================================*/

/**
 * @brief Fault Test: Null Read
 */
static void test_fault_portal_null_read(void)
{
	int portalid;
	int local;

	local = knode_get_num();

	test_assert((portalid = kportal_create(local, 0)) >= 0);

		test_assert(kportal_read(portalid, NULL, PORTAL_SIZE) == -EINVAL);

	test_assert(kportal_unlink(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Invalid Write                                                  *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Write
 */
static void test_fault_portal_invalid_write(void)
{
	char buffer[PORTAL_SIZE];

	test_assert(kportal_write(-1, buffer, PORTAL_SIZE) == -EINVAL);
	test_assert(kportal_write(KPORTAL_MAX, buffer, PORTAL_SIZE) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Bad Write                                                      *
 *============================================================================*/

/**
 * @brief Fault Test: Bad Write
 */
static void test_fault_portal_bad_write(void)
{
	int local;
	int portalid;
	char buffer[PORTAL_SIZE];

	local = knode_get_num();

	test_assert((portalid = kportal_create(local, 0)) >= 0);

		test_assert(kportal_write(portalid, buffer, PORTAL_SIZE) == -EBADF);

	test_assert(kportal_unlink(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Invalid Write Size                                             *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Write Size
 */
static void test_fault_portal_invalid_write_size(void)
{
	int local;
	int remote;
	int portalid;
	char buffer[PORTAL_SIZE];

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_open(local, remote, 0)) >= 0);

		test_assert(kportal_write(portalid, buffer, -1) == -EINVAL);
		test_assert(kportal_write(portalid, buffer, 0) == -EINVAL);
		test_assert(kportal_write(portalid, buffer, KPORTAL_MAX_SIZE + 1) == -EINVAL);

	test_assert(kportal_close(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Null Write                                                     *
 *============================================================================*/

/**
 * @brief Fault Test: Null Write
 */
static void test_fault_portal_null_write(void)
{
	int local;
	int remote;
	int portalid;

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	test_assert((portalid = kportal_open(local, remote, 0)) >= 0);

		test_assert(kportal_write(portalid, NULL, PORTAL_SIZE) == -EINVAL);

	test_assert(kportal_close(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Invalid Wait                                                   *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid Wait
 */
static void test_fault_portal_invalid_wait(void)
{
	test_assert(kportal_wait(-1) == -EINVAL);
	test_assert(kportal_wait(KPORTAL_MAX) == -EINVAL);
}

/*============================================================================*
 * Fault Test: Invalid ioctl                                                  *
 *============================================================================*/

/**
 * @brief Fault Test: Invalid ioctl
 */
static void test_fault_portal_invalid_ioctl(void)
{
	int local;
	int portalid;
	size_t volume;
	uint64_t latency;

	test_assert(kportal_ioctl(-1, KPORTAL_IOCTL_GET_VOLUME, &volume) == -EINVAL);
	test_assert(kportal_ioctl(-1, KPORTAL_IOCTL_GET_LATENCY, &latency) == -EINVAL);
	test_assert(kportal_ioctl(KPORTAL_MAX, KPORTAL_IOCTL_GET_VOLUME, &volume) == -EINVAL);
	test_assert(kportal_ioctl(KPORTAL_MAX, KPORTAL_IOCTL_GET_LATENCY, &latency) == -EINVAL);

	local = knode_get_num();

	test_assert((portalid = kportal_create(local, 0)) >=  0);

		test_assert(kportal_ioctl(portalid, -1, &volume) == -ENOTSUP);
		test_assert(kportal_ioctl(portalid, KPORTAL_IOCTL_GET_VOLUME, NULL) == -EFAULT);
		test_assert(kportal_ioctl(portalid, KPORTAL_IOCTL_GET_LATENCY, NULL) == -EFAULT);

	test_assert(kportal_unlink(portalid) == 0);
}

/*============================================================================*
 * Fault Test: Bad portalid                                                   *
 *============================================================================*/

/**
 * @brief Fault Test: Bad portalid
 */
static void test_fault_portal_bad_portalid(void)
{
	int portal;
	int remote;
	int local;
	size_t volume;
	char buffer[PORTAL_SIZE];

	local = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	portal = 0;

	test_assert(kportal_close(portal) == -EBADF);
	test_assert(kportal_unlink(portal) == -EBADF);
	test_assert(kportal_allow(portal, remote, 0) == -EBADF)
	test_assert(kportal_read(portal, buffer, PORTAL_SIZE) == -EBADF);
	test_assert(kportal_write(portal, buffer, PORTAL_SIZE) == -EBADF);
	test_assert(kportal_wait(portal) == -EBADF);
	test_assert(kportal_ioctl(portal, KPORTAL_IOCTL_GET_VOLUME, &volume) == -EBADF);
}

/*============================================================================*
 * Stress Tests                                                               *
 *============================================================================*/

/*============================================================================*
 * Stress Test: Portal Create Unlink                                          *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Create Unlink
 */
PRIVATE void test_stress_portal_create_unlink(void)
{
	int local;
	int portalid;

	local = knode_get_num();

	for (int i = 0; i < NSETUPS; ++i)
	{
		test_assert((portalid = kportal_create(local, 0)) >= 0);
		test_assert(kportal_unlink(portalid) == 0);
	}
}

/*============================================================================*
 * Stress Test: Portal Open Close                                             *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Open Close
 */
PRIVATE void test_stress_portal_open_close(void)
{
	int local;
	int remote;
	int portalid;

	local  = knode_get_num();
	remote = (local == MASTER_NODENUM) ? SLAVE_NODENUM : MASTER_NODENUM;

	for (int i = 0; i < NSETUPS; ++i)
	{
		test_assert((portalid = kportal_open(local, remote, 0)) >= 0);
		test_assert(kportal_close(portalid) == 0);
	}
}

/*============================================================================*
 * Stress Test: Portal Broadcast                                              *
 *============================================================================*/

PRIVATE void test_stress_do_sender_single(int local, int remote)
{
	int portalid;

	message[0] = local;

	for (int i = 0; i < NSETUPS; ++i)
	{
		test_assert((portalid = kportal_open(local, remote, 0)) >= 0);

		for (int j = 0; j < NCOMMUNICATIONS; ++j)
			test_assert(kportal_write(portalid, message, PORTAL_SIZE) == PORTAL_SIZE);

		test_assert(kportal_close(portalid) == 0);
	}
}

PRIVATE void test_stress_do_receiver_single(int local, int remote)
{
	int portalid;

	for (int i = 0; i < NSETUPS; ++i)
	{
		test_assert((portalid = kportal_create(local, 0)) >= 0);

		for (int j = 0; j < NCOMMUNICATIONS; ++j)
		{
			message[0] = local;
			test_assert(kportal_allow(portalid, remote, 0) >= 0);
			test_assert(kportal_read(portalid, message, PORTAL_SIZE) == PORTAL_SIZE);
			test_assert(message[0] == remote);
		}

		test_assert(kportal_unlink(portalid) == 0);
	}
}

/**
 * @brief Stress Test: Portal Broadcast
 */
PRIVATE void test_stress_portal_broadcast(void)
{
	if (knode_get_num() == MASTER_NODENUM)
		test_stress_do_sender_single(MASTER_NODENUM, SLAVE_NODENUM);
	else
		test_stress_do_receiver_single(SLAVE_NODENUM, MASTER_NODENUM);
}

/*============================================================================*
 * Stress Test: Portal Gather                                                 *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Gather
 */
PRIVATE void test_stress_portal_gather(void)
{
	if (knode_get_num() == MASTER_NODENUM)
		test_stress_do_receiver_single(MASTER_NODENUM, SLAVE_NODENUM);
	else
		test_stress_do_sender_single(SLAVE_NODENUM, MASTER_NODENUM);
}

/*============================================================================*
 * Stress Test: Portal Ping-Pong                                              *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Ping-Pong
 */
PRIVATE void test_stress_portal_pingpong(void)
{
	int local;
	int remote;
	int inportal;
	int outportal;

	local = knode_get_num();
	remote = local == MASTER_NODENUM ? SLAVE_NODENUM : MASTER_NODENUM;

	for (int i = 0; i < NSETUPS; ++i)
	{
		test_assert((inportal = kportal_create(local, 0)) >= 0);
		test_assert((outportal = kportal_open(local, remote, 0)) >= 0);

		if (local == MASTER_NODENUM)
		{
			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				message[0] = local;
				test_assert(kportal_allow(inportal, remote, 0) >= 0);
				test_assert(kportal_read(inportal, message, PORTAL_SIZE) == PORTAL_SIZE);
				test_assert(message[0] == remote);

				message[0] = local;
				test_assert(kportal_write(outportal, message, PORTAL_SIZE) == PORTAL_SIZE);
			}
		}
		else
		{
			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				message[0] = local;
				test_assert(kportal_write(outportal, message, PORTAL_SIZE) == PORTAL_SIZE);

				test_assert(kportal_allow(inportal, remote, 0) >= 0);
				test_assert(kportal_read(inportal, message, PORTAL_SIZE) == PORTAL_SIZE);
				test_assert(message[0] == remote);
			}
		}

		test_assert(kportal_close(outportal) == 0);
		test_assert(kportal_unlink(inportal) == 0);
	}
}

/*============================================================================*
 * Stress Test: Portal Multiplexing Broadcast                                 *
 *============================================================================*/

PRIVATE void test_stress_do_sender_multiplex(int local, int remote)
{
	int portalids[TEST_MULTIPLEXATION_PORTAL_PAIRS];

	message[0] = local;

	for (int i = 0; i < NSETUPS; ++i)
	{
		for (int j = 0; j < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++j)
			test_assert((portalids[j] = kportal_open(local, remote, j)) >= 0);

		for (int j = 0; j < NCOMMUNICATIONS; ++j)
			for (int k = 0; k < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++k)
				test_assert(kportal_write(portalids[k], message, PORTAL_SIZE) == PORTAL_SIZE);

		for (int j = 0; j < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++j)
			test_assert(kportal_close(portalids[j]) == 0);
	}
}

PRIVATE void test_stress_do_receiver_multiplex(int local, int remote)
{
	int portalids[TEST_MULTIPLEXATION_PORTAL_PAIRS];

	for (int i = 0; i < NSETUPS; ++i)
	{
		for (int j = 0; j < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++j)
			test_assert((portalids[j] = kportal_create(local, j)) >= 0);

		for (int j = 0; j < NCOMMUNICATIONS; ++j)
		{
			for (int k = 0; k < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++k)
			{
				message[0] = local;
				test_assert(kportal_allow(portalids[k], remote, k) >= 0);
				test_assert(kportal_read(portalids[k], message, PORTAL_SIZE) == PORTAL_SIZE);
				test_assert(message[0] == remote);
			}
		}

		for (int j = 0; j < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++j)
			test_assert(kportal_unlink(portalids[j]) == 0);
	}
}

/**
 * @brief Stress Test: Portal Multiplexing Broadcast
 */
PRIVATE void test_stress_portal_multiplexing_broadcast(void)
{
	if (knode_get_num() == MASTER_NODENUM)
		test_stress_do_sender_multiplex(MASTER_NODENUM, SLAVE_NODENUM);
	else
		test_stress_do_receiver_multiplex(SLAVE_NODENUM, MASTER_NODENUM);
}

/*============================================================================*
 * Stress Test: Portal Multiplexing Gather                                    *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Multiplexing Gather
 */
PRIVATE void test_stress_portal_multiplexing_gather(void)
{
	if (knode_get_num() == MASTER_NODENUM)
		test_stress_do_receiver_multiplex(MASTER_NODENUM, SLAVE_NODENUM);
	else
		test_stress_do_sender_multiplex(SLAVE_NODENUM, MASTER_NODENUM);
}

/*============================================================================*
 * Stress Test: Portal Multiplexing Ping-Pong                                 *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Multiplexing Ping-Pong
 */
PRIVATE void test_stress_portal_multiplexing_pingpong(void)
{
	int local;
	int remote;
	int inportals[TEST_MULTIPLEXATION_PORTAL_PAIRS];
	int outportals[TEST_MULTIPLEXATION_PORTAL_PAIRS];

	local = knode_get_num();
	remote = local == MASTER_NODENUM ? SLAVE_NODENUM : MASTER_NODENUM;

	for (int i = 0; i < NSETUPS; ++i)
	{
		for (int j = 0; j < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++j)
		{
			test_assert((inportals[j] = kportal_create(local, j)) >= 0);
			test_assert((outportals[j] = kportal_open(local, remote, j)) >= 0);
		}

		if (local == SLAVE_NODENUM)
		{
			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++k)
				{
					message[0] = local;
					test_assert(kportal_allow(inportals[k], remote, k) >= 0);
					test_assert(kportal_read(inportals[k], message, PORTAL_SIZE) == PORTAL_SIZE);
					test_assert(message[0] == remote);

					message[0] = local;
					test_assert(kportal_write(outportals[k], message, PORTAL_SIZE) == PORTAL_SIZE);
				}
			}
		}
		else
		{
			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++k)
				{
					message[0] = local;
					test_assert(kportal_write(outportals[k], message, PORTAL_SIZE) == PORTAL_SIZE);

					message[0] = local;
					test_assert(kportal_allow(inportals[k], remote, k) >= 0);
					test_assert(kportal_read(inportals[k], message, PORTAL_SIZE) == PORTAL_SIZE);
					test_assert(message[0] == remote);
				}
			}
		}

		for (int j = 0; j < TEST_MULTIPLEXATION_PORTAL_PAIRS; ++j)
		{
			test_assert(kportal_close(outportals[j]) == 0);
			test_assert(kportal_unlink(inportals[j]) == 0);
		}
	}
}

/*============================================================================*
 * Stress Test: Thread synchronization                                        *
 *============================================================================*/

/*============================================================================*
 * Stress Test: Portal Thread Multiplexing Broadcast                          *
 *============================================================================*/

PRIVATE void test_stress_do_sender_thread(int tid, int local, int remote)
{
	int nports;
	int portalids[TEST_THREAD_PORTALID_NUM];

	for (int i = 0; i < NSETUPS; ++i)
	{
		nports = 0;
		for (int j = 0; j < TEST_THREAD_NPORTS; ++j)
		{
			if (j == (tid + nports * TEST_THREAD_MAX))
				test_assert((portalids[nports++] = kportal_open(local, remote, j)) >= 0);

			nanvix_fence(&_fence);
		}

		for (int j = 0; j < NCOMMUNICATIONS; ++j)
		{
			for (int k = 0; k < nports; ++k)
				test_assert(kportal_write(portalids[k], message_out, PORTAL_SIZE) == PORTAL_SIZE);

			nanvix_fence(&_fence);
		}

		for (int j = 0; j < nports; ++j)
			test_assert(kportal_close(portalids[j]) == 0);

		nanvix_fence(&_fence);
	}
}

PRIVATE void test_stress_do_receiver_thread(int tid, int local, int remote)
{
	int nports;
	int portalids[TEST_THREAD_PORTALID_NUM];
	char * msg;

	msg = message_in[tid];

	for (int i = 0; i < NSETUPS; ++i)
	{
		nports = 0;
		for (int j = 0; j < TEST_THREAD_NPORTS; ++j)
		{
			if (j == (tid + nports * TEST_THREAD_MAX))
				test_assert((portalids[nports++] = kportal_create(local, j)) >= 0);

			nanvix_fence(&_fence);
		}

		for (int j = 0; j < NCOMMUNICATIONS; ++j)
		{
			for (int k = 0; k < nports; ++k)
			{
				kmemset(msg, -1, PORTAL_SIZE);
				test_assert(kportal_allow(portalids[k], remote, (tid + k * TEST_THREAD_MAX)) >= 0);
				test_assert(kportal_read(portalids[k], msg, PORTAL_SIZE) == PORTAL_SIZE);
				for (int l = 0; l < PORTAL_SIZE; ++l)
					test_assert(msg[l] == remote);
			}

			nanvix_fence(&_fence);
		}

		for (int j = 0; j < nports; ++j)
			test_assert(kportal_unlink(portalids[j]) == 0);

		nanvix_fence(&_fence);
	}
}

PRIVATE void * do_thread_multiplexing_broadcast(void * arg)
{
	if (knode_get_num() == MASTER_NODENUM)
		test_stress_do_sender_thread(((int)(intptr_t) arg), MASTER_NODENUM, SLAVE_NODENUM);
	else
		test_stress_do_receiver_thread(((int)(intptr_t) arg), SLAVE_NODENUM, MASTER_NODENUM);

	return (NULL);
}

/**
 * @brief Stress Test: Portal Thread Multiplexing Broadcast
 */
PRIVATE void test_stress_portal_thread_multiplexing_broadcast(void)
{
	kthread_t tid[TEST_THREAD_MAX - 1];

	kmemset(message_out, (char) knode_get_num(), PORTAL_SIZE);
	nanvix_fence_init(&_fence, TEST_THREAD_MAX);

	/* Create threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_create(&tid[i - 1], do_thread_multiplexing_broadcast, ((void *)(intptr_t) i)) == 0);

	do_thread_multiplexing_broadcast(0);

	/* Join threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_join(tid[i - 1], NULL) == 0);
}

/*============================================================================*
 * Stress Test: Portal Thread Multiplexing Gather                             *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Thread Multiplexing Gather
 */
PRIVATE void * do_thread_multiplexing_gather(void * arg)
{
	if (knode_get_num() == MASTER_NODENUM)
		test_stress_do_receiver_thread(((int)(intptr_t) arg), MASTER_NODENUM, SLAVE_NODENUM);
	else
		test_stress_do_sender_thread(((int)(intptr_t) arg), SLAVE_NODENUM, MASTER_NODENUM);

	return (NULL);
}

/**
 * @brief Stress Test: Portal Thread Multiplexing Gather
 */
PRIVATE void test_stress_portal_thread_multiplexing_gather(void)
{
	kthread_t tid[TEST_THREAD_MAX - 1];

	kmemset(message_out, (char) knode_get_num(), PORTAL_SIZE);
	nanvix_fence_init(&_fence, TEST_THREAD_MAX);

	/* Create threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_create(&tid[i - 1], do_thread_multiplexing_gather, ((void *)(intptr_t) i)) == 0);

	do_thread_multiplexing_gather(0);

	/* Join threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_join(tid[i - 1], NULL) == 0);
}

/*============================================================================*
 * Stress Test: Portal Thread Multiplexing Ping-Pong                          *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Thread Multiplexing Ping-Pong
 */
PRIVATE void * do_thread_multiplexing_pingpong(void * arg)
{
	int tid;
	int local;
	int remote;
	int nports;
	int inportals[TEST_THREAD_PORTALID_NUM];
	int outportals[TEST_THREAD_PORTALID_NUM];
	char * msg;

	tid = ((int)(intptr_t) arg);
	msg = message_in[tid];

	local  = knode_get_num();
	remote = local == MASTER_NODENUM ? SLAVE_NODENUM : MASTER_NODENUM;

	for (int i = 0; i < NSETUPS; ++i)
	{
		nports = 0;
		for (int j = 0; j < TEST_THREAD_NPORTS; ++j)
		{
			if (j == (tid + nports * TEST_THREAD_MAX))
			{
				test_assert((inportals[nports] = kportal_create(local, j)) >= 0);
				test_assert((outportals[nports] = kportal_open(local, remote, j)) >= 0);
				nports++;
			}

			nanvix_fence(&_fence);
		}

		if (local == MASTER_NODENUM)
		{
			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < nports; ++k)
				{
					kmemset(msg, -1, PORTAL_SIZE);
					test_assert(kportal_allow(inportals[k], remote, (tid + k * TEST_THREAD_MAX)) >= 0);
					test_assert(kportal_read(inportals[k], msg, PORTAL_SIZE) == PORTAL_SIZE);
					for (int l = 0; l < PORTAL_SIZE; ++l)
						test_assert(msg[l] == remote);

					test_assert(kportal_write(outportals[k], message_out, PORTAL_SIZE) == PORTAL_SIZE);
				}

				nanvix_fence(&_fence);
			}
		}
		else
		{
			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < nports; ++k)
				{
					test_assert(kportal_write(outportals[k], message_out, PORTAL_SIZE) == PORTAL_SIZE);

					kmemset(msg, -1, PORTAL_SIZE);
					test_assert(kportal_allow(inportals[k], remote, (tid + k * TEST_THREAD_MAX)) >= 0);
					test_assert(kportal_read(inportals[k], msg, PORTAL_SIZE) == PORTAL_SIZE);
					for (int l = 0; l < PORTAL_SIZE; ++l)
						test_assert(msg[l] == remote);
				}

				nanvix_fence(&_fence);
			}
		}

		for (int j = 0; j < nports; ++j)
		{
			test_assert(kportal_close(outportals[j]) == 0);
			test_assert(kportal_unlink(inportals[j]) == 0);
		}

		nanvix_fence(&_fence);
	}

	return (NULL);
}

/**
 * @brief Stress Test: Portal Thread Multiplexing Ping-Pong
 */
PRIVATE void test_stress_portal_thread_multiplexing_pingpong(void)
{
	kthread_t tid[TEST_THREAD_MAX - 1];

	kmemset(message_out, (char) knode_get_num(), PORTAL_SIZE);
	nanvix_fence_init(&_fence, TEST_THREAD_MAX);

	/* Create threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_create(&tid[i - 1], do_thread_multiplexing_pingpong, ((void *)(intptr_t) i)) == 0);

	do_thread_multiplexing_pingpong(0);

	/* Join threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_join(tid[i - 1], NULL) == 0);
}

/*============================================================================*
 * Stress Test: Portal Thread Multiplexing Affinity                           *
 *============================================================================*/

#define PPORT(x) (tid + x * TEST_THREAD_MAX)

/**
 * @brief Stress Test: Portal Thread Multiplexing Affinity
 */
PRIVATE void * do_thread_multiplexing_affinity(void * arg)
{
	int tid;
	int local;
	int remote;
	int nports;
	int inportals[TEST_THREAD_PORTALID_NUM];
	int outportals[TEST_THREAD_PORTALID_NUM];
	char * msg;

	test_assert(kthread_set_affinity(TEST_THREAD_AFFINITY) > 0);
	test_assert(core_get_id() == TEST_CORE_AFFINITY);

	tid = ((int)(intptr_t) arg);
	msg = message_in[tid];

	local  = knode_get_num();
	remote = local == MASTER_NODENUM ? SLAVE_NODENUM : MASTER_NODENUM;

	for (int i = 0; i < NSETUPS_AFFINITY; ++i)
	{
		nports = 0;
		for (int j = 0; j < TEST_THREAD_NPORTS; ++j)
		{
			if (j == (tid + nports * TEST_THREAD_MAX))
			{
				test_assert((inportals[nports] = kportal_create(local, j)) >= 0);
				test_assert((outportals[nports] = kportal_open(local, remote, j)) >= 0);
				nports++;
			}

			nanvix_fence(&_fence);
		}

		if (local == MASTER_NODENUM)
		{
			for (int j = 0; j < NCOMMS_AFFINITY; ++j)
			{
				for (int k = 0; k < nports; ++k)
				{
					kmemset(msg, -1, PORTAL_SIZE);
					test_assert(kportal_allow(inportals[k], remote, (tid + k * TEST_THREAD_MAX)) >= 0);
					test_assert(kportal_read(inportals[k], msg, PORTAL_SIZE) == PORTAL_SIZE);
					for (int l = 0; l < PORTAL_SIZE; ++l)
						test_assert(msg[l] == remote);

					test_assert(kportal_write(outportals[k], message_out, PORTAL_SIZE) == PORTAL_SIZE);
				}

				nanvix_fence(&_fence);
			}
		}
		else
		{
			for (int j = 0; j < NCOMMS_AFFINITY; ++j)
			{
				for (int k = 0; k < nports; ++k)
				{
					test_assert(kportal_write(outportals[k], message_out, PORTAL_SIZE) == PORTAL_SIZE);

					kmemset(msg, -1, PORTAL_SIZE);
					test_assert(kportal_allow(inportals[k], remote, (tid + k * TEST_THREAD_MAX)) >= 0);
					test_assert(kportal_read(inportals[k], msg, PORTAL_SIZE) == PORTAL_SIZE);
					for (int l = 0; l < PORTAL_SIZE; ++l)
						test_assert(msg[l] == remote);
				}

				nanvix_fence(&_fence);
			}
		}

		for (int j = 0; j < nports; ++j)
		{
			test_assert(kportal_close(outportals[j]) == 0);
			test_assert(kportal_unlink(inportals[j]) == 0);
		}

		nanvix_fence(&_fence);
	}

	return (NULL);
}

/**
 * @brief Stress Test: Portal Thread Multiplexing Affinity
 */
PRIVATE void test_stress_portal_thread_multiplexing_affinity(void)
{
	kthread_t tid[TEST_THREAD_MAX - 1];

	kmemset(message_out, (char) knode_get_num(), PORTAL_SIZE);
	nanvix_fence_init(&_fence, TEST_THREAD_MAX);

	/* Create threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_create(&tid[i - 1], do_thread_multiplexing_affinity, ((void *)(intptr_t) i)) == 0);

	do_thread_multiplexing_affinity(0);

	/* Join threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_join(tid[i - 1], NULL) == 0);

	test_assert(kthread_set_affinity(KTHREAD_AFFINITY_DEFAULT) == TEST_THREAD_AFFINITY);
}

/*============================================================================*
 * Stress Test: Portal Thread Multiplexing Broadcast Local                    *
 *============================================================================*/

PRIVATE void * do_thread_multiplexing_broadcast_local(void * arg)
{
	int tid;
	int local;
	int nports;
	int portalids[TEST_THREAD_NPORTS];
	char * msg;

	tid = ((int)(intptr_t) arg);
	msg = message_in[tid];

	local = knode_get_num();

	if (tid == 0)
	{
		for (int i = 0; i < NSETUPS; ++i)
		{
			nports = 0;
			for (int j = 0; j < (TEST_THREAD_NPORTS - 1); ++j)
			{
				test_assert((portalids[nports++] = kportal_open(local, local, j)) >= 0);

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < nports; ++k)
					test_assert(kportal_write(portalids[k], message_out, PORTAL_SIZE) == PORTAL_SIZE);

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < nports; ++j)
				test_assert(kportal_close(portalids[j]) == 0);

			nanvix_fence(&_fence);
		}
	}
	else
	{
		for (int i = 0; i < NSETUPS; ++i)
		{
			nports = 0;
			for (int j = 0; j < (TEST_THREAD_NPORTS - 1); ++j)
			{
				if (j == ((tid - 1) + nports * (TEST_THREAD_MAX - 1)))
					test_assert((portalids[nports++] = kportal_create(local, j)) >= 0);

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < nports; ++k)
				{
					kmemset(msg, -1, PORTAL_SIZE);
					test_assert(kportal_allow(portalids[k], local, ((tid - 1) + k * (TEST_THREAD_MAX - 1))) >= 0);
					test_assert(kportal_read(portalids[k], msg, PORTAL_SIZE) == PORTAL_SIZE);
					for (int l = 0; l < PORTAL_SIZE; ++l)
						test_assert(msg[l] == 1);
				}

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < nports; ++j)
				test_assert(kportal_unlink(portalids[j]) == 0);

			nanvix_fence(&_fence);
		}
	}

	return (NULL);
}

/**
 * @brief Stress Test: Portal Thread Multiplexing Broadcast Local
 */
PRIVATE void test_stress_portal_thread_multiplexing_broadcast_local(void)
{
	kthread_t tid[TEST_THREAD_MAX - 1];

	kmemset(message_out, 1, PORTAL_SIZE);
	nanvix_fence_init(&_fence, TEST_THREAD_MAX);

	/* Create threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_create(&tid[i - 1], do_thread_multiplexing_broadcast_local, ((void *)(intptr_t) i)) == 0);

	do_thread_multiplexing_broadcast_local(0);

	/* Join threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_join(tid[i - 1], NULL) == 0);
}

/*============================================================================*
 * Stress Test: Portal Thread Multiplexing Gather Local                       *
 *============================================================================*/

/**
 * @brief Stress Test: Portal Thread Multiplexing Gather Local
 */
PRIVATE void * do_thread_multiplexing_gather_local(void * arg)
{
	int tid;
	int local;
	int nports;
	int portalids[TEST_THREAD_NPORTS];
	char * msg;

	tid = ((int)(intptr_t) arg);
	msg = message_in[tid];

	local = knode_get_num();

	if (tid != 0)
	{
		for (int i = 0; i < NSETUPS; ++i)
		{
			nports = 0;
			for (int j = 0; j < (TEST_THREAD_NPORTS - 1); ++j)
			{
				if (j == ((tid - 1) + nports * (TEST_THREAD_MAX - 1)))
					test_assert((portalids[nports++] = kportal_open(local, local, j)) >= 0);

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < nports; ++k)
					test_assert(kportal_write(portalids[k], message_out, PORTAL_SIZE) == PORTAL_SIZE);

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < nports; ++j)
				test_assert(kportal_close(portalids[j]) == 0);

			nanvix_fence(&_fence);
		}
	}
	else
	{
		for (int i = 0; i < NSETUPS; ++i)
		{
			nports = 0;
			for (int j = 0; j < (TEST_THREAD_NPORTS - 1); ++j)
			{
				test_assert((portalids[nports++] = kportal_create(local, j)) >= 0);

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < NCOMMUNICATIONS; ++j)
			{
				for (int k = 0; k < nports; ++k)
				{
					kmemset(msg, -1, PORTAL_SIZE);
					test_assert(kportal_allow(portalids[k], local, k) >= 0);
					test_assert(kportal_read(portalids[k], msg, PORTAL_SIZE) == PORTAL_SIZE);
					for (int l = 0; l < PORTAL_SIZE; ++l)
						test_assert(msg[l] == 1);
				}

				nanvix_fence(&_fence);
			}

			for (int j = 0; j < nports; ++j)
				test_assert(kportal_unlink(portalids[j]) == 0);

			nanvix_fence(&_fence);
		}
	}

	return (NULL);
}

/**
 * @brief Stress Test: Portal Thread Multiplexing Gather
 */
PRIVATE void test_stress_portal_thread_multiplexing_gather_local(void)
{
	kthread_t tid[TEST_THREAD_MAX - 1];

	kmemset(message_out, 1, PORTAL_SIZE);
	nanvix_fence_init(&_fence, TEST_THREAD_MAX);

	/* Create threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_create(&tid[i - 1], do_thread_multiplexing_gather_local, ((void *)(intptr_t) i)) == 0);

	do_thread_multiplexing_gather_local(0);

	/* Join threads. */
	for (int i = 1; i < TEST_THREAD_MAX; ++i)
		test_assert(kthread_join(tid[i - 1], NULL) == 0);
}

/*============================================================================*
 * Test Driver                                                                *
 *============================================================================*/

/**
 * @brief API tests.
 */
static struct test portal_tests_api[] = {
	{ test_api_portal_create_unlink,          "[test][portal][api] portal create unlink          [passed]" },
	{ test_api_portal_open_close,             "[test][portal][api] portal open close             [passed]" },
	{ test_api_portal_get_volume,             "[test][portal][api] portal get volume             [passed]" },
	{ test_api_portal_get_latency,            "[test][portal][api] portal get latency            [passed]" },
	{ test_api_portal_get_counters,           "[test][portal][api] portal get counters           [passed]" },
	{ test_api_portal_read_write,             "[test][portal][api] portal read write             [passed]" },
	{ test_api_portal_read_write_large,       "[test][portal][api] portal read write large       [passed]" },
	{ test_api_portal_virtualization,         "[test][portal][api] portal virtualization         [passed]" },
	{ test_api_portal_multiplexation,         "[test][portal][api] portal multiplexation         [passed]" },
	{ test_api_portal_allow,                  "[test][portal][api] portal allow                  [passed]" },
	{ test_api_portal_multiplexation_2,       "[test][portal][api] portal multiplexation 2       [passed]" },
	{ test_api_portal_multiplexation_3,       "[test][portal][api] portal multiplexation 3       [passed]" },
	{ test_api_portal_multiplexation_4,       "[test][portal][api] portal multiplexation 4       [passed]" },
	{ test_api_portal_multiplexation_2_large, "[test][portal][api] portal multiplexation 2 Large [passed]" },
	{ test_api_portal_multiplexation_3_large, "[test][portal][api] portal multiplexation 3 Large [passed]" },
	{ test_api_portal_multiplexation_4_large, "[test][portal][api] portal multiplexation 4 Large [passed]" },
	{ test_api_portal_pending_msg_unlink,     "[test][portal][api] portal pending msg unlink     [passed]" },
	{ test_api_portal_msg_forwarding,         "[test][portal][api] portal message forwarding     [passed]" },
#if __NANVIX_USE_COMM_WITH_TASKS
	{ test_api_portal_on_dispatcher,          "[test][portal][api] portal with tasks             [passed]" },
#endif
	{ NULL,                                    NULL                                                        },
};

/**
 * @brief Fault tests.
 */
static struct test portal_tests_fault[] = {
	{ test_fault_portal_invalid_create,     "[test][portal][fault] portal invalid create     [passed]" },
	{ test_fault_portal_double_create,      "[test][portal][fault] portal double create      [passed]" },
	{ test_fault_portal_invalid_unlink,     "[test][portal][fault] portal invalid unlink     [passed]" },
	{ test_fault_portal_double_unlink,      "[test][portal][fault] portal double unlink      [passed]" },
	{ test_fault_portal_bad_unlink,         "[test][portal][fault] portal bad unlink         [passed]" },
	{ test_fault_portal_invalid_open,       "[test][portal][fault] portal invalid open       [passed]" },
	{ test_fault_portal_invalid_close,      "[test][portal][fault] portal invalid close      [passed]" },
	{ test_fault_portal_double_close,       "[test][portal][fault] portal double close       [passed]" },
	{ test_fault_portal_bad_close,          "[test][portal][fault] portal bad close          [passed]" },
	{ test_fault_portal_invalid_allow,      "[test][portal][fault] portal invalid allow      [passed]" },
	{ test_fault_portal_double_allow,       "[test][portal][fault] portal double allow       [passed]" },
	{ test_fault_portal_bad_allow,          "[test][portal][fault] portal bad allow          [passed]" },
	{ test_fault_portal_invalid_read,       "[test][portal][fault] portal invalid read       [passed]" },
	{ test_fault_portal_bad_read,           "[test][portal][fault] portal bad read           [passed]" },
	{ test_fault_portal_invalid_read_size,  "[test][portal][fault] portal invalid read size  [passed]" },
	{ test_fault_portal_null_read,          "[test][portal][fault] portal null read          [passed]" },
	{ test_fault_portal_invalid_write,      "[test][portal][fault] portal invalid write      [passed]" },
	{ test_fault_portal_bad_write,          "[test][portal][fault] portal bad write          [passed]" },
	{ test_fault_portal_invalid_write_size, "[test][portal][fault] portal invalid write size [passed]" },
	{ test_fault_portal_null_write,         "[test][portal][fault] portal null write         [passed]" },
	{ test_fault_portal_invalid_wait,       "[test][portal][fault] portal invalid wait       [passed]" },
	{ test_fault_portal_invalid_ioctl,      "[test][portal][fault] portal invalid ioctl      [passed]" },
	{ test_fault_portal_bad_portalid,       "[test][portal][fault] portal bad portalid       [passed]" },
	{ NULL,                                  NULL                                                      },
};

/**
 * @brief Stress tests.
 */
PRIVATE struct test portal_tests_stress[] = {
	/* Intra-Cluster API Tests */
	{ test_stress_portal_create_unlink,                       "[test][portal][stress] portal create unlink                       [passed]" },
	{ test_stress_portal_open_close,                          "[test][portal][stress] portal open close                          [passed]" },
	{ test_stress_portal_broadcast,                           "[test][portal][stress] portal broadcast                           [passed]" },
	{ test_stress_portal_gather,                              "[test][portal][stress] portal gather                              [passed]" },
	{ test_stress_portal_pingpong,                            "[test][portal][stress] portal ping-pong                           [passed]" },
	{ test_stress_portal_multiplexing_broadcast,              "[test][portal][stress] portal multiplexing broadcast              [passed]" },
	{ test_stress_portal_multiplexing_gather,                 "[test][portal][stress] portal multiplexing gather                 [passed]" },
	{ test_stress_portal_multiplexing_pingpong,               "[test][portal][stress] portal multiplexing ping-pong              [passed]" },
	{ test_stress_portal_thread_multiplexing_broadcast,       "[test][portal][stress] portal thread multiplexing broadcast       [passed]" },
	{ test_stress_portal_thread_multiplexing_gather,          "[test][portal][stress] portal thread multiplexing gather          [passed]" },
	{ test_stress_portal_thread_multiplexing_pingpong,        "[test][portal][stress] portal thread multiplexing ping-pong       [passed]" },
#if (CORE_SUPPORTS_MULTITHREADING && __NANVIX_MICROKERNEL_DYNAMIC_SCHED)
	{ test_stress_portal_thread_multiplexing_affinity,        "[test][portal][stress] portal thread multiplexing affinity        [passed]" },
#endif
	{ test_stress_portal_thread_multiplexing_broadcast_local, "[test][portal][stress] portal thread multiplexing broadcast local [passed]" },
	{ test_stress_portal_thread_multiplexing_gather_local,    "[test][portal][stress] portal thread multiplexing gather local    [passed]" },
	{ NULL,                                                    NULL                                                                        },
};

/**
 * The test_thread_mgmt() function launches testing units on thread manager.
 *
 * @author Pedro Henrique Penna
 */
void test_portal(void)
{
	int local;

	local = knode_get_num();

	kmemset(message_out, (char) local, PORTAL_SIZE);

	if (local == MASTER_NODENUM || local == SLAVE_NODENUM)
	{
		/* API Tests */
		if (local == MASTER_NODENUM)
			nanvix_puts("--------------------------------------------------------------------------------");
		for (unsigned i = 0; portal_tests_api[i].test_fn != NULL; i++)
		{
			portal_tests_api[i].test_fn();

			test_barrier_nodes();

			if (local == MASTER_NODENUM)
				nanvix_puts(portal_tests_api[i].name);
		}

		/* Fault Tests */
		if (local == MASTER_NODENUM)
			nanvix_puts("--------------------------------------------------------------------------------");
		for (unsigned i = 0; portal_tests_fault[i].test_fn != NULL; i++)
		{
			portal_tests_fault[i].test_fn();

			if (local == MASTER_NODENUM)
				nanvix_puts(portal_tests_fault[i].name);
		}

		/* Stress Tests */
		if (local == MASTER_NODENUM)
			nanvix_puts("--------------------------------------------------------------------------------");
		for (unsigned i = 0; portal_tests_stress[i].test_fn != NULL; i++)
		{
			portal_tests_stress[i].test_fn();

			test_barrier_nodes();

			if (local == MASTER_NODENUM)
				nanvix_puts(portal_tests_stress[i].name);
		}
	}
}

#endif /* __TARGET_HAS_PORTAL */
