/*
 * EnclaveFuzz - SGX Enclave Fuzzing Test Harness (Auto-Generated)
 *
 * Generated from EDL: TestEnclave.edl
 *
 * ============================================================================
 * Fuzzing Framework Architecture
 * ============================================================================
 *
 * Initialization (once):
 *     LibFuzzer → LLVMFuzzerInitialize()
 *                  ↓
 *                 customized_init()  ← Register harnesses, calculate weights
 *
 * Fuzzing loop (per input):
 *     LibFuzzer → LLVMFuzzerTestOneInput(data, size)
 *                  ↓ Reinitialize g_fdp with new input
 *                  ↓ Recreate enclave (__g_harness_eid)
 *                  ↓
 *                 customized_harness()  ← Weighted selection
 *                  ↓
 *                 _harness_xxx()   ← Auto-generated test functions
 *                  ↓
 *                 ECall → Enclave Code
 *
 * ============================================================================
 * EDL Attribute Reference
 * ============================================================================
 *
 * | Attribute    | Meaning             | Fuzzing Strategy (ECall)         |
 * |--------------|---------------------|----------------------------------|
 * | [in]         | Input to callee     | Generate fuzzy data (Host→Encl)  |
 * | [out]        | Output from callee  | Allocate buffer (Encl→Host)      |
 * | [in,out]     | Bidirectional       | Generate input + allocate        |
 * | [size=N]     | Buffer size (bytes) | Use N for allocation             |
 * | [count=N]    | Array element count | Use N * sizeof(element)          |
 * | [string]     | Null-terminated str | Ensure null terminator           |
 * | [user_check] | No auto checking    | High fuzz value                  |
 *
 * CRITICAL: Direction Semantics ([in]/[out] relative to callee)
 * - For ECalls (Enclave is callee):
 *   [in] = Host→Enclave → FUZZ THIS in harness
 *   [out] = Enclave→Host → Allocate buffer only
 * - For OCalls (Host is callee):
 *   [in] = Enclave→Host → No fuzzing needed
 *   [out] = Host→Enclave → FUZZ THIS in OCall wrapper
 *
 * ============================================================================
 * Memory Management (Two Approaches)
 * ============================================================================
 * Approach 1 (Auto-Managed by g_alloc_mgr) - CURRENT DEFAULT:
 * - Use calloc() + g_alloc_mgr.push_back() to track allocations
 * - Framework in LLVMFuzzerTestOneInput (at test.cpp) automatically frees all
 * tracked memory after each iteration
 * - No explicit free() needed in harness functions
 * - Pros: Simple, no memory leaks, centralized cleanup
 * - Cons: Memory accumulates until end of iteration
 *
 * Approach 2 (Explicit free()):
 * - Use calloc() without g_alloc_mgr tracking
 * - Manually write free() calls at appropriate locations in harness code
 * - Pros: Immediate memory release, lower memory footprint
 * - Cons: Must ensure all allocations are freed, risk of memory leaks
 *
 * Usage: Choose approach based on your needs:
 * - Default: g_alloc_mgr for safety and simplicity
 * - Manual: Direct free() for memory-sensitive scenarios
 *
 * ============================================================================
 * Weighted Selection System
 * ============================================================================
 * Each harness has a weight (default: 10). Adjust weights in customized_init():
 * - High weight (e.g., 50-100) for critical/bottleneck paths
 * - Low weight (e.g., 1-5) for well-covered paths
 * - Modify test_harness_registry[i].weight before calculating total_weight
 *
 * ============================================================================
 */

#include "FuzzedDataProvider.h"
#include "TestEnclave_u.h"
#include <errno.h>
#include <sgx_urts.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// ============================================================================
// Global Variables
// ============================================================================

extern FuzzedDataProvider *g_fdp;
extern std::vector<uint8_t *> g_alloc_mgr;
extern sgx_enclave_id_t __g_harness_eid;

// Fuzzing configuration parameters
static size_t g_max_strlen = 128; // Max string length for [string] attributes
static size_t g_max_cnt = 32;     // Max count for unbounded arrays
static size_t g_max_size = 512;   // Max size for unbounded buffers

// ============================================================================
// Test Harness Registration System
// ============================================================================

typedef void (*TestHarness)(void);

struct TestHarnessEntry {
  TestHarness function;
  int weight; // Selection weight (default: 10)
};

static TestHarnessEntry test_harness_registry[10240];
static unsigned int test_harness_count = 0;
static int total_weight = 0;

// ============================================================================
// OCall Wrappers
// ============================================================================
// These wrappers intercept OCalls and fuzz [out] parameters
// to test Enclave's resilience to untrusted data
// ============================================================================

extern "C" void _harness_uprint(const char *str) { uprint(str); }

extern "C" void _harness_usgx_exit(int reason) { usgx_exit(reason); }

extern "C" int _harness_ucreate_thread(void) {
  int _fuzz_ret = ucreate_thread();
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" void _harness_u_sgxssl_ftime(void *timeptr, uint32_t timeb_len) {
  u_sgxssl_ftime(timeptr, timeb_len);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    size_t count_0_timeptr = ((1) * (timeb_len)) / 1;
    g_fdp->ConsumeData((void *)timeptr, count_0_timeptr * 1);
  }
}

extern "C" size_t _harness_u_sgxssl_write(int fd, const void *buf, size_t n) {
  size_t _fuzz_ret = u_sgxssl_write(fd, buf, n);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(size_t));
  }
  return _fuzz_ret;
}

extern "C" size_t _harness_u_sgxssl_read(int fd, void *buf, size_t count) {
  size_t _fuzz_ret = u_sgxssl_read(fd, buf, count);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    size_t count_0_buf = ((1) * (count)) / 1;
    g_fdp->ConsumeData((void *)buf, count_0_buf * 1);
  }
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(size_t));
  }
  return _fuzz_ret;
}

extern "C" int _harness_u_sgxssl_close(int fd) {
  int _fuzz_ret = u_sgxssl_close(fd);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" void _harness_sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf) {
  sgx_oc_cpuidex(cpuinfo, leaf, subleaf);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    for (size_t i_0_0 = 0; i_0_0 < 4; i_0_0++) {
      g_fdp->ConsumeData(&cpuinfo[i_0_0], sizeof(int));
    }
  }
}

extern "C" int
_harness_sgx_thread_wait_untrusted_event_ocall(const void *self) {
  int _fuzz_ret = sgx_thread_wait_untrusted_event_ocall(self);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    size_t count_0_self =
        g_fdp->ConsumeIntegralInRange<size_t>(1 < 8 ? (20 / 1) : 1, g_max_cnt);
    g_fdp->ConsumeData((void *)self, count_0_self * 1);
  }
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" int
_harness_sgx_thread_set_untrusted_event_ocall(const void *waiter) {
  int _fuzz_ret = sgx_thread_set_untrusted_event_ocall(waiter);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    size_t count_0_waiter =
        g_fdp->ConsumeIntegralInRange<size_t>(1 < 8 ? (20 / 1) : 1, g_max_cnt);
    g_fdp->ConsumeData((void *)waiter, count_0_waiter * 1);
  }
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" int
_harness_sgx_thread_setwait_untrusted_events_ocall(const void *waiter,
                                                   const void *self) {
  int _fuzz_ret = sgx_thread_setwait_untrusted_events_ocall(waiter, self);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    size_t count_0_waiter =
        g_fdp->ConsumeIntegralInRange<size_t>(1 < 8 ? (20 / 1) : 1, g_max_cnt);
    g_fdp->ConsumeData((void *)waiter, count_0_waiter * 1);
  }
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    size_t count_0_self =
        g_fdp->ConsumeIntegralInRange<size_t>(1 < 8 ? (20 / 1) : 1, g_max_cnt);
    g_fdp->ConsumeData((void *)self, count_0_self * 1);
  }
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" int
_harness_sgx_thread_set_multiple_untrusted_events_ocall(const void **waiters,
                                                        size_t total) {
  int _fuzz_ret =
      sgx_thread_set_multiple_untrusted_events_ocall(waiters, total);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" int _harness_pthread_wait_timeout_ocall(unsigned long long waiter,
                                                   unsigned long long timeout) {
  int _fuzz_ret = pthread_wait_timeout_ocall(waiter, timeout);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" int _harness_pthread_create_ocall(unsigned long long self) {
  int _fuzz_ret = pthread_create_ocall(self);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

extern "C" int _harness_pthread_wakeup_ocall(unsigned long long waiter) {
  int _fuzz_ret = pthread_wakeup_ocall(waiter);
  if (g_fdp->ConsumeProbability<double>() < 0.5 /* as an example */) {
    g_fdp->ConsumeData(&_fuzz_ret, sizeof(int));
  }
  return _fuzz_ret;
}

// ============================================================================
// ECall Test Harnesses
// ============================================================================
// Auto-generated harness functions for each ECall
// Each function prepares fuzz inputs and invokes the corresponding ECall
// ============================================================================

static void _harness_t_sgxssl_call_apis(void) {
  t_sgxssl_call_apis(__g_harness_eid);
}
static void _harness_new_thread_func(void) { new_thread_func(__g_harness_eid); }

// ============================================================================
// Customized Initialization
// ============================================================================
// This function is called once during fuzzer initialization
// (LLVMFuzzerInitialize).
//
// REQUIRED: Register all test harnesses by filling test_harness_registry[]
//
// Usage:
//   test_harness_registry[test_harness_count++] = {harness_function, weight};
//
// IMPORTANT:
// - This function is called BEFORE any fuzzing iterations start
// - DO NOT create or initialize the enclave here (__g_harness_eid will be 0)
// - DO NOT access g_fdp here (it's not initialized yet)
// - Keep initialization lightweight and fast
// - Weight MUST be > 0 for all harnesses
//
// Optional: Add custom initialization such as:
// - Environment variable configuration (setenv, putenv)
// - Global state initialization
// - Logging/debugging setup
// - Resource pre-allocation
// - Configuration file loading
// ============================================================================

extern "C" void customized_init() {
  // ========================================================================
  // Step 1: Register all test harnesses
  // ========================================================================
  test_harness_registry[test_harness_count++] = {_harness_t_sgxssl_call_apis,
                                                 10}; // Test t_sgxssl_call_apis
  test_harness_registry[test_harness_count++] = {_harness_new_thread_func,
                                                 10}; // Test new_thread_func

  // ========================================================================
  // Step 2: Calculate total weight for weighted random selection
  // ========================================================================

  // Sanity check: ensure at least one harness is registered
  if (test_harness_count == 0) {
    fprintf(stderr, "[!] Error: No test harnesses registered\n");
    abort();
  }

  total_weight = 0;
  for (unsigned int i = 0; i < test_harness_count; i++) {
    total_weight += test_harness_registry[i].weight;
  }

  // Sanity check: ensure total weight > 0
  if (total_weight == 0) {
    fprintf(stderr, "[!] Error: All harness weights are 0\n");
    abort();
  }

  // ========================================================================
  // Step 3: Custom initialization (optional)
  // ========================================================================
  // Examples:
  // - setenv("SGX_AESM_ADDR", "1", 1);
  // - freopen("/tmp/fuzzer.log", "w", stderr);
  // - Initialize global variables
  // - Pre-load configuration files
}

// ============================================================================
// Main Test Entry Point
// ============================================================================
// Called by LLVMFuzzerTestOneInput for each fuzzing iteration
// Performs weighted random selection of test harnesses
// ============================================================================

extern "C" void customized_harness(void) {
  // Weighted random selection
  do {
    int rand_val = g_fdp->ConsumeIntegralInRange<int>(0, total_weight - 1);
    int cumulative = 0;
    for (unsigned int i = 0; i < test_harness_count; i++) {
      cumulative += test_harness_registry[i].weight;
      if (rand_val < cumulative) {
        test_harness_registry[i].function();
        break;
      }
    }
  } while (g_fdp->remaining_bytes() > 0);
}
