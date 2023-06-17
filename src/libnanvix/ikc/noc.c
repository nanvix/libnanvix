/*
 * MIT License
 *
 * Copyright(c) 2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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

#include <nanvix/kernel/kernel.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/portal.h>
#include <nanvix/sys/sync.h>
#include <posix/errno.h>

#include "task.h"

/*============================================================================*
 * knode_get_num()                                                            *
 *============================================================================*/

/*
 * @see kernel_node_get_num()
 */
int knode_get_num(void)
{
	int ret;

	ret = kcall0(
		NR_node_get_num
	);

	return (ret);
}

/*============================================================================*
 * kcluster_get_num()                                                         *
 *============================================================================*/

/*
 * @see kernel_cluster_get_num()
 */
int kcluster_get_num(void)
{
	int ret;

	ret = kcall0(
		NR_cluster_get_num
	);

	return (ret);
}

/*============================================================================*
 * kcomm_get_port()                                                           *
 *============================================================================*/

/*
 * @see kernel_comm_get_num()
 */
int kcomm_get_port(int id, int type)
{
	int ret;

	if (id < 0)
		return (-EINVAL);

	if ((type != COMM_TYPE_MAILBOX) && (type != COMM_TYPE_PORTAL))
		return (-EINVAL);

#if __NANVIX_IKC_USES_ONLY_MAILBOX
	if (type == COMM_TYPE_PORTAL)
		return (kportal_get_port(id));
#endif

	ret = kcall2(
		NR_comm_get_port,
		(word_t) id,
		(word_t) type
	);

	return (ret);
}

/*============================================================================*
 * knoc_init()                                                                *
 *============================================================================*/

/**
 * @details The knoc_init() Initializes underlying noc systems.
 */
PUBLIC void knoc_init(void)
{
    kprintf("[user][noc] initializing the noc system");

	#if __TARGET_HAS_PORTAL || (__TARGET_HAS_MAILBOX && __NANVIX_IKC_USES_ONLY_MAILBOX)
		kportal_init();
	#endif

	#if __TARGET_HAS_SYNC || (__TARGET_HAS_MAILBOX && __NANVIX_IKC_USES_ONLY_MAILBOX)
		ksync_init();
	#endif

	#if __TARGET_HAS_MAILBOX
		kmailbox_init();
	#endif

	#if __NANVIX_USE_COMM_WITH_TASKS
		ikc_flow_init();
	#endif
}
