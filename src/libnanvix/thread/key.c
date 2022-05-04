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
 * kthread_key_create()                                                       *
 *============================================================================*/

PUBLIC int kthread_key_create(kthread_key_t * key, void (* destructor)(void *))
{
	int ret;

	ret = kcall2(
		NR_thread_key_create,
		(word_t) key,
		(word_t) destructor
	);

	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_key_delete()                                                       *
 *============================================================================*/

PUBLIC int kthread_key_delete(kthread_key_t key)
{
	int ret;

	ret = kcall1(
		NR_thread_key_delete,
		(word_t) key
	);

	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_getspecific()                                                      *
 *============================================================================*/

PUBLIC int kthread_getspecific(kthread_key_t key, void ** value)
{

	int ret, tid;

	tid = kthread_self();

	ret = kcall3(
		NR_thread_getspecific,
		(word_t) tid,
		(word_t) key,
		(word_t) value
	);

	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}

/*============================================================================*
 * kthread_setspecific()                                                      *
 *============================================================================*/

PUBLIC int kthread_setspecific(kthread_key_t key, void * value)
{

	int ret, tid;

	tid = kthread_self();

	ret = kcall3(
		NR_thread_setspecific,
		(word_t) tid,
		(word_t) key,
		(word_t) value
	);

	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}


PUBLIC int kthread_key_exit(int * retv)
{

	int tid, ret;

	tid = kthread_self();

	ret = kcall2(
		NR_thread_setspecific,
		(word_t) tid,
		(word_t) retv
	);

	if (ret < 0)
	{
		errno = -ret;
		return (-1);
	}

	return (ret);
}
