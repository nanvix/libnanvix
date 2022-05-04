/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
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
#include <nanvix/sys/thread.h>
#include <posix/errno.h>

/*============================================================================*
 * kthread_self()                                                             *
 *============================================================================*/

/*
 * @see sys_kthread_get_id()
 */
kthread_t kthread_self(void)
{
	return (kcall0(NR_thread_get_id));
}

/*============================================================================*
 * kthread_create()                                                           *
 *============================================================================*/

/*
 * @see sys_kthread_create()
 */
int kthread_create(
	kthread_t *tid,
	void *(*start)(void*),
	void *arg
)
{
	int ret;

	ret = kcall3(
		NR_thread_create,
		(word_t) tid,
		(word_t) start,
		(word_t) arg
	);

	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_exit()                                                             *
 *============================================================================*/

/*
 * @see sys_kthread_exit().
 */
int kthread_exit(void *retval)
{
	int ret;

	ret = kcall1(
		NR_thread_exit,
		(word_t) retval
	);
	
	kprintf("oi 3");
	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_join()                                                             *
 *============================================================================*/

/*
 * @see sys_kthread_join()
 */
int kthread_join(
	kthread_t tid,
	void **retval
)
{
	int ret;

	ret = kcall2(
		NR_thread_join,
		(word_t) tid,
		(word_t) retval
	);

	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_yield()                                                            *
 *============================================================================*/

/*
 * @see sys_kthread_yield()
 */
int kthread_yield(void)
{
	int ret;

	ret = kcall0(NR_thread_yield);

	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_set_affinity()                                                     *
 *============================================================================*/

/*
 * @see kernel_thread_set_affinity()
 */
int kthread_set_affinity(int new_affinity)
{
	int ret;

	/* Invalid affinity. */
	if (!KTHREAD_AFFINITY_IS_VALID(new_affinity))
	{
		errno = EINVAL;
		return (-1);
	}

	ret = kcall1(
		NR_thread_set_affinity,
		(word_t) new_affinity
	);

	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_stats()                                                            *
 *============================================================================*/

/*
 * @see sys_kthread_stats()
 */
int kthread_stats(
	kthread_t tid,
	uint64_t * buffer,
	int stat
)
{
	int ret;

	ret = kcall3(
		NR_thread_stats,
		(word_t) tid,
		(word_t) buffer,
		(word_t) stat
	);

	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * ksleep()                                                                   *
 *============================================================================*/

/*
 * @see sys_sleep()
 */
int ksleep(void)
{
	int ret;

	ret = kcall0(NR_sleep);

	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kwakeup()                                                                  *
 *============================================================================*/

/*
 * @see sys_wakeup()
 */
int kwakeup(kthread_t tid)
{
	int ret;

	ret = kcall1(
		NR_wakeup,
		(word_t) tid
	);

	/* System call failed. */
	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}
