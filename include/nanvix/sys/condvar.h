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

#ifndef NANVIX_SYS_CONDVAR_H_
#define NANVIX_SYS_CONDVAR_H_

#include <nanvix/kernel/kernel.h>

#if (CORES_NUM > 1)

	#include <nanvix/sys/mutex.h>
	#include <nanvix/sys/thread.h>

	/**
	 * @brief Condition Variable
	 */
	struct nanvix_cond_var
	{
		spinlock_t lock;
		#if (__NANVIX_CONDVAR_SLEEP)
			kthread_t tids[THREAD_MAX];
		#else
			bool locked;
			int waiting;
		#endif
	};

	/**
	 * @brief Initializes a condition variable
	 * @param cond Condition variable to be initialized
	 * @return 0 if the condition variable was sucessfully initialized
	 * or a negative code error if an error occurred
	 */
	extern int nanvix_cond_init(struct nanvix_cond_var *cond);

	/**
	 * @brief Block thread on a condition variable
	 * @param cond Condition variable to wait for
	 * @param mutex Mutex unlocked when waiting for a signal and locked when
	 * the funtion returns after a cond_signal call
	 * @return 0 upon successfull completion or a negative error code
	 * upon failure
	 */
	extern int nanvix_cond_wait(struct nanvix_cond_var *cond, struct nanvix_mutex *mutex);

	/**
	 * @brief Unclock thread blocked on a condition variable
	 * @param cond Condition variable to signal
	 * @return 0 upon successfull completion or a negative error code
	 * upon failure
	 */
	extern int nanvix_cond_signal(struct nanvix_cond_var *cond);

	/**
	 * @brief Destroy a condition variable
	 * @param cond Condition variable to destroy
	 * @return 0 upon successfull completion or a negative error code
	 * upon failure
	 */
	extern int nanvix_condvar_destroy(struct nanvix_cond_var *cond);

#endif  /* CORES_NUM */

#endif  /* NANVIX_SYS_CONDVAR_H_ */

/**@}*/