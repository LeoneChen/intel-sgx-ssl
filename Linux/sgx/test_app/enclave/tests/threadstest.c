#if defined(__cplusplus)
extern "C"{
#endif
void SGXSanLogEnter(const char *str);
#if defined(__cplusplus)
}
#endif
#define LogEnter SGXSanLogEnter
/* ====================================================================
 * Copyright (c) 2016 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */

#include <stdio.h>

#include <openssl/crypto.h>
#include "threads.h"

#include <sgx.h>
#include <stdbool.h>

#define fprintf(stream, msg...) printf(msg)

sgx_status_t ucreate_thread();

void (*func)(void) = NULL;
volatile int busy_wait = 0;

#if !defined(OPENSSL_THREADS) || defined(CRYPTO_TDEBUG)

typedef unsigned int thread_t;

static int run_thread(thread_t *t, void (*f)(void))
{
       int res = 0;
       func = f;
       busy_wait = 1;
       sgx_status_t status = ucreate_thread(&res);
       if (status != SGX_SUCCESS || res != 0)
               return 0;
       return 1;
}

static int wait_for_thread(thread_t thread)
{
	while (busy_wait == 1);
	return 1;
}

#else

typedef pthread_t thread_t;

/*
static void *thread_run(void *arg)
{
    void (*f)(void);
    *(void **) (&f) = arg;
    f();
    return NULL;
}
*/

static int run_thread(thread_t *t, void (*f)(void))
{
	int res = 0;
	func = f;
	busy_wait = 1;
	sgx_status_t status = ucreate_thread(&res);
	if (status != SGX_SUCCESS || res != 0)
		return 0;
	return 1;
    // return pthread_create(t, NULL, thread_run, *(void **) &f) == 0;
}

static int wait_for_thread(thread_t thread)
{
	while (busy_wait == 1);
	return 1;
	//return pthread_join(thread, NULL) == 0;
}

#endif

void new_thread_func()
{
    LogEnter(__func__);
	printf("in new thread, id: %llu\n",sgx_thread_self());
    if (func)
	func();
	busy_wait = 0;
}

static int test_lock(void)
{
    CRYPTO_RWLOCK *lock = CRYPTO_THREAD_lock_new();

    if (!CRYPTO_THREAD_read_lock(lock)) {
        fprintf(stderr, "CRYPTO_THREAD_read_lock() failed\n");
        return 0;
    }

    if (!CRYPTO_THREAD_unlock(lock)) {
        fprintf(stderr, "CRYPTO_THREAD_unlock() failed\n");
        return 0;
    }

    CRYPTO_THREAD_lock_free(lock);

    return 1;
}

static CRYPTO_ONCE once_run = CRYPTO_ONCE_STATIC_INIT;
static unsigned once_run_count = 0;

static void once_do_run(void)
{
    once_run_count++;
}

static void once_run_thread_cb(void)
{
    CRYPTO_THREAD_run_once(&once_run, once_do_run);
}

static int test_once(void)
{
    thread_t thread;
    if (!run_thread(&thread, once_run_thread_cb) ||
        !wait_for_thread(thread))
    {
        fprintf(stderr, "run_thread() failed\n");
        return 0;
    }

    if (!CRYPTO_THREAD_run_once(&once_run, once_do_run)) {
        fprintf(stderr, "CRYPTO_THREAD_run_once() failed\n");
        return 0;
    }

    if (once_run_count != 1) {
        fprintf(stderr, "once run %u times\n", once_run_count);
        return 0;
    }

    return 1;
}

static CRYPTO_THREAD_LOCAL thread_local_key;
static unsigned destructor_run_count = 0;
static int thread_local_thread_cb_ok = 0;

static void thread_local_destructor(void *arg)
{
    unsigned *count;

    if (arg == NULL)
        return;

    count = arg;

    (*count)++;
}

static void thread_local_thread_cb(void)
{
    void *ptr;

    ptr = CRYPTO_THREAD_get_local(&thread_local_key);
    if (ptr != NULL) {
        fprintf(stderr, "ptr not NULL\n");
        return;
    }

    if (!CRYPTO_THREAD_set_local(&thread_local_key, &destructor_run_count)) {
        fprintf(stderr, "CRYPTO_THREAD_set_local() failed\n");
        return;
    }

    ptr = CRYPTO_THREAD_get_local(&thread_local_key);
    if (ptr != &destructor_run_count) {
        fprintf(stderr, "invalid ptr\n");
        return;
    }

    thread_local_thread_cb_ok = 1;
}

static int test_thread_local(void)
{
    thread_t thread;
    void *ptr = NULL;

    if (!CRYPTO_THREAD_init_local(&thread_local_key, thread_local_destructor)) {
        fprintf(stderr, "CRYPTO_THREAD_init_local() failed\n");
        return 0;
    }

    ptr = CRYPTO_THREAD_get_local(&thread_local_key);
    if (ptr != NULL) {
        fprintf(stderr, "ptr not NULL\n");
        return 0;
    }

    if (!run_thread(&thread, thread_local_thread_cb) ||
        !wait_for_thread(thread))
    {
        fprintf(stderr, "run_thread() failed\n");
        return 0;
    }

    if (thread_local_thread_cb_ok != 1) {
        fprintf(stderr, "thread-local thread callback failed\n");
        return 0;
    }

#if defined(OPENSSL_THREADS) && !defined(CRYPTO_TDEBUG)

    ptr = CRYPTO_THREAD_get_local(&thread_local_key);
    if (ptr != NULL) {
        fprintf(stderr, "ptr not NULL\n");
        return 0;
    }

	// not supported in enclave - no atexit function
    //if (destructor_run_count != 1) {
    //    fprintf(stderr, "thread-local destructor run %u times\n",
    //            destructor_run_count);
    //    return 0;
    //}

#endif
    if (!CRYPTO_THREAD_cleanup_local(&thread_local_key)) {
        fprintf(stderr, "CRYPTO_THREAD_cleanup_local() failed\n");
        return 0;
    }

    return 1;
}

int threads_test()
{
	printf("original thread, id: %llu\n",sgx_thread_self());
    
    if (!test_lock())
      return 1;

    if (!test_once())
      return 1;

    if (!test_thread_local())
      return 1;

    printf("PASS\n");
    return 0;
}

