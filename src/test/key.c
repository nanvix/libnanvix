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

#include <nanvix/sys/thread.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/runtime/fence.h>
#include "test.h"

#define BIG_VALUE 64
#define FIRST_VALUE  0xaabbccdd

/*
 * @brief Fence variable used in tests.
 */
PRIVATE struct nanvix_fence fence;

/*
 * @brief Auxiliar of synchronizations.
 */
PRIVATE int nsyncs;

PRIVATE void test_key_destroyer(void * arg)
{
	UNUSED(arg);
}

PRIVATE void test_api_key_init(void)
{
	kthread_key_t keys[5];

	test_assert(kthread_key_create(&keys[0], &test_key_destroyer) == 0);
	test_assert(kthread_key_create(&keys[1], NULL) == 0);

	test_assert(kthread_key_delete(keys[0])== 0);
	test_assert(kthread_key_delete(keys[1])== 0);
	
	for (int i = 0; i < 4; i++)
		test_assert(kthread_key_create(&keys[i], &test_key_destroyer) == 0);
	
	test_assert(kthread_key_create(&keys[5], &test_key_destroyer) < 0);
	
	for (int j = 0; j < 4; j++)
		test_assert(kthread_key_delete(keys[j]) == 0);

	test_assert(kthread_key_delete(keys[5]) < 0);
}

PRIVATE void test_api_getset_key(void)
{
	kthread_key_t key;
	void * value[2];
	*value = (void *) FIRST_VALUE;

	test_assert(kthread_key_create(&key, NULL) == 0);

	test_assert(kthread_setspecific(key, value[0]) == 0);
	test_assert(kthread_getspecific(key, &value[1]) == 0);

	test_assert(value[0] == value[1]);

	test_assert(kthread_key_delete(key) == 0);

}

PRIVATE void test_fault_key_init(void)
{
	test_assert(kthread_key_create(NULL, &test_key_destroyer) < 0);
	test_assert(kthread_key_create(NULL, NULL) < 0);

	test_assert(kthread_key_delete(BIG_VALUE) < 0);

}

PRIVATE void test_fault_get(void)
{
	void * value;

	test_assert(kthread_getspecific(BIG_VALUE, &value) < 0);
	test_assert(kthread_getspecific(BIG_VALUE, NULL) < 0);
}

PRIVATE void test_fault_set(void)
{
	void * value;

	test_assert(kthread_setspecific(BIG_VALUE, &value) < 0);
	test_assert(kthread_setspecific(BIG_VALUE, NULL) < 0);
}

struct test_args
{
	kthread_key_t key;
	void * dummy;

} arg_tests[NTHREADS];

PRIVATE void * task_create(void * arg)
{
	int args = *((int *) arg); 

	test_assert(kthread_setspecific(arg_tests[args].key, &arg_tests[args].dummy) == 0);

	test_assert(nanvix_fence(&fence) == 0);	

	test_assert(kthread_getspecific(arg_tests[args].key, &arg_tests[args].dummy) == 0);
	
	return (NULL);
}

PRIVATE	void test_stress_key_getset(void)
{	
#if (THREAD_MAX > 2)
	
	kthread_t tids[NTHREADS];
	nsyncs = 0;
	
	test_assert(nanvix_fence_init(&fence, NTHREADS) == 0);

		for (int i = 0; i < THREAD_KEY_MAX; i++)
			test_assert(kthread_key_create(&arg_tests[i].key, NULL) == 0);

		for (int i = 0; i < NTHREADS; i++)
			test_assert(kthread_create(&tids[i], task_create, (void *) &i) == 0);
		
		for (int i = 0; i < NTHREADS; i++)
			test_assert(kthread_join(tids[i], NULL) == 0);

		for (int i = 0; i < THREAD_KEY_MAX; i++)
			test_assert(kthread_key_delete(arg_tests[i].key) == 0);

	test_assert(nanvix_fence_destroy(&fence) == 0);
		
#endif
}

/**
 * @brief API tests.
 */
PRIVATE struct test key_mgmt_tests_api[] = {
	{ test_api_key_init,               "[test][key][api] key create/delete                  [passed]" },
	{ test_api_getset_key,             "[test][thread][api] key getspecific/ setspecific    [passed]" },
	{ NULL,                            NULL                                                           },
};

PRIVATE struct test key_mgmt_tests_fault[] = {
	{ test_fault_key_init,              "[test][key][fault] key create/delete               [passed]" },
	{ test_fault_get,                   "[test][key][fault] key getspecific                 [passed]" },
	{ test_fault_set,                   "[test][key][fault] key setspecific                 [passed]" },
	{ NULL,                             NULL                                                          },
};

PRIVATE struct test key_mgmt_tests_stress[] = {
	{ test_stress_key_getset,            "[test][key][stress] key get/setspecific           [passed]" },
	{ NULL,                               NULL                                                        },
};

PUBLIC void test_key(void)
{
	nanvix_puts("--------------------------------------------------------------------------------");
	for (int i = 0; key_mgmt_tests_api[i].test_fn != NULL; i++)
	{
		key_mgmt_tests_api[i].test_fn();
		nanvix_puts(key_mgmt_tests_api[i].name);
	}

	nanvix_puts("--------------------------------------------------------------------------------");
	for (int i = 0; key_mgmt_tests_fault[i].test_fn != NULL; i++)
	{
		key_mgmt_tests_fault[i].test_fn();
		nanvix_puts(key_mgmt_tests_fault[i].name);
	}

	nanvix_puts("--------------------------------------------------------------------------------");
	for (int i = 0; key_mgmt_tests_stress[i].test_fn != NULL; i++)
	{
		key_mgmt_tests_stress[i].test_fn();
		nanvix_puts(key_mgmt_tests_stress[i].name);
	}
}

