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
#include "test.h"

#define NKEYS 32
#define BIG_VALUE 64

PRIVATE void test_key_destroyer(void * arg)
{
	UNUSED(arg);
}

PRIVATE void test_api_key_init(void)
{
	kthread_key_t keys[2];

	test_assert(kthread_key_create(&keys[0], &test_key_destroyer) == 0);
	test_assert(kthread_key_create(&keys[1], NULL) == 0);

	test_assert(kthread_key_delete(keys[0])== 0);
	test_assert(kthread_key_delete(keys[1])== 0);
//adicionar teste criar 4 keys + 1
}

PRIVATE void test_api_getset_key(void)
{
	kthread_key_t key;
	void * value;

	test_assert(kthread_key_create(&key, NULL) == 0);

	test_assert(kthread_setspecific(key, NULL) == 0);
	test_assert(kthread_getspecific(key, &value) == 0);

	test_assert(value == NULL);

	test_assert(kthread_key_delete(key)== 0);

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

PRIVATE void test_stress_key_overflow(void)
{
#if (THREAD_MAX > 1)

	kthread_key_t keys[NKEYS + 1];

	for (int i = 0; i < NKEYS; i++)
		test_assert(kthread_key_create(&keys[i], NULL) == 0);

	test_assert(kthread_key_create(&keys[NKEYS], NULL) < 0);

	for (int j = 0; j < NKEYS; j++)
		test_assert(kthread_key_delete(keys[j]) == 0);
#endif
}

PRIVATE void test_stress_key_getset(void)
{
#if __TARGET_HAS_SYNC

	kthread_key_t keys[NKEYS];
	void * values[NKEYS];
	void * results[NKEYS];

	int nodes[PROCESSOR_CLUSTERS_NUM];
	barrier_t barrier;

	barrier = barrier_create(nodes, PROCESSOR_CLUSTERS_NUM);

	for (int i = 0; i < NKEYS; i++)
	{
		test_assert(kthread_key_create(&keys[i], NULL) == 0);
		test_assert(kthread_setspecific(keys[i], &values[i]) == 0);
	}

	barrier_wait(barrier);

	for (int j = 0; j < NKEYS; j++)
	{
		test_assert(kthread_getspecific(keys[j], &results[j]) == 0);
		test_assert(* results[j] == * values[j]);
	}

	for (int k = 0; k < NKEYS; k++)
		test_assert(kthread_key_delete(keys[k]) == 0);

	test_assert(barrier_destroy(barrier) == 0);

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
	{ test_stress_key_overflow,          "[test][key][stress] key overflow                  [passed]" },
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

