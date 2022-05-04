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

/**
 * @addtogroup nanvix Nanvix System
 */
/**@{*/

#ifndef NANVIX_SYS_THREAD_H_
#define NANVIX_SYS_THREAD_H_

	#include <nanvix/kernel/kernel.h>

	/**
	 * @brief Thread ID.
	 */
	typedef int kthread_t;

	/**
	 * @name Thread Management Kernel Calls
	 */
	/**@{*/
	extern kthread_t kthread_self(void);
	extern int kthread_create(kthread_t *, void *(*)(void*), void *);
	extern int kthread_exit(void *);
	extern int kthread_join(kthread_t, void **);
	extern int kthread_yield(void);
	extern int kthread_set_affinity(int);
	extern int kthread_stats(kthread_t, uint64_t *, int);
	/**@}*/

	/**
	 * @brief Thread key.
	 */
	typedef int kthread_key_t;

	/**
	 * @name Thread Key Kernel Calls
	 */
	/**@{*/
	extern  int kthread_key_create(kthread_key_t *, void (*)(void *));
	extern  int kthread_key_delete(kthread_key_t);
	extern  int kthread_getspecific(kthread_key_t, void **);
	extern	int kthread_setspecific(kthread_key_t, void *);
	extern  int kthread_key_exit(int *);
	/**@}*/

	/**
	 * @name Thread Synchronization Kernel Calls
	 */
	/**@{*/
	extern int ksleep(void);
	extern int kwakeup(kthread_t);
	/**@}*/

	/**
	 * @brief Shutdowns the kernel.
	 *
	 * @returns Upon successful completion, this function does not
	 * return.Upon failure, a negative error code is returned instead.
	 */
	extern int kshutdown(void);

	/**
	 * @brief Terminates the calling process.
	 *
	 * @param status Exit status.
	 *
	 * @note This function does not return.
	 */
	extern void _kexit(int status);

#endif /* NANVIX_SYS_THREAD_H_ */

/**@}*/
