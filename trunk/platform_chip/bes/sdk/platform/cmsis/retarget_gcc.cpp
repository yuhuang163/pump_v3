/***************************************************************************
 *
 * Copyright 2015-2024 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#if (defined(__GNUC__) && !defined(__ARMCC_VERSION))

#if !defined(NOSTD) && !defined(NO_LIBC) && !defined(SEMIHOST_ENABLE)
#define LIBC_HOOKS
#endif

#ifdef  LIBC_HOOKS

#include <errno.h>
#include "hal_trace.h"

#ifndef FILEHANDLE
typedef int FILEHANDLE;
#endif

#define WEAK     __attribute__((weak))
#define PACKED   __attribute__((packed))

#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/syslimits.h>
#define PREFIX(x)    x

extern "C" int WEAK PREFIX(_write)(FILEHANDLE fh, const unsigned char *buffer,
                                    unsigned int length, int mode)
{
    int n = 0; // n is the number of bytes written
    if (fh < 3) {
        hal_trace_output(buffer, length);
        n = length;
    }
    return n;
}

extern "C" int WEAK PREFIX(_read)(FILEHANDLE fh, unsigned char *buffer,
                                     unsigned int length, int mode)
{
    int n = 0; // n is the number of bytes read
    if (fh < 3) {
        // only read a character at a time from stdin
        // TODO: Read from trace uart input
        n = 1;
    }
    return n;
}

#if defined(__GNUC__)
/* prevents the exception handling name demangling code getting pulled in */
namespace __gnu_cxx {
    void __verbose_terminate_handler() {
        ASSERT(0, "Exception");
    }
}
extern "C" WEAK void __cxa_pure_virtual(void);
extern "C" WEAK void __cxa_pure_virtual(void)
{
    _exit(1);
}

#endif

// Provide implementation of _sbrk (low-level dynamic memory allocation
// routine) for GCC_ARM which compares new heap pointer with MSP instead of
// SP.  This make it compatible with RTX RTOS thread stacks.
#if defined(__GNUC__) && defined(__arm__)
// Linker defined symbol used by _sbrk to indicate where heap should start.
extern "C" int __end__;
extern "C" uint32_t  __HeapLimit;

// Turn off the errno macro and use actual global variable instead.
#undef errno
extern "C" int errno;

// For ARM7 only
register unsigned char * stack_ptr __asm ("sp");

// Dynamic memory allocation related syscall.
extern "C" caddr_t _sbrk(int incr)
{
    static unsigned char* heap = (unsigned char*)&__end__;
    unsigned char*        prev_heap = heap;
    unsigned char*        new_heap = heap + incr;

#if defined(TARGET_ARM7)
    if (new_heap >= stack_ptr) {
#elif defined(TARGET_CORTEX_A)
    if (new_heap >= (unsigned char*)&__HeapLimit) {     /* __HeapLimit is end of heap section */
#else
    if (new_heap >= (unsigned char*)&__HeapLimit) {     /* __HeapLimit is end of heap section */
#endif

        ASSERT(false, "_sbrk:Heap overflowed: start=%p end=%p cur=%p incr=%d",
            (unsigned char*)&__end__, (unsigned char*)&__HeapLimit, heap, incr);
        errno = ENOMEM;
        return (caddr_t)-1;
    }

    TRACE(5, "_sbrk: start=%p end=%p cur=%p incr=0x%02x remain 0x%x",
            (unsigned char*)&__end__, (unsigned char*)&__HeapLimit, heap, incr, (unsigned char*)&__HeapLimit - heap);

    heap = new_heap;
    return (caddr_t) prev_heap;
}
#endif


#if defined(__GNUC__) && defined(__arm__)
extern "C" WEAK void _exit(int return_code)
{
#else
namespace std {
extern "C" void exit(int return_code)
{
#endif
    if (return_code) {
        ASSERT(false, "system die: %d", return_code);
    }

    do { volatile int i = 0; i++; } while (1);
}

#if !(defined(__GNUC__) && defined(__arm__)) && !defined(TOOLCHAIN_GCC_CW)
} //namespace std
#endif

extern "C" WEAK int _getpid(void)
{
    return 1;
}

extern "C" WEAK int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

extern "C" WEAK int _close(int file)
{
    return -1;
}

extern "C" WEAK int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

extern "C" WEAK int _isatty(int file)
{
    return 1;
}

extern "C" WEAK int _lseek(int file, int ptr, int dir)
{
    return 0;
}

extern "C" WEAK int _open(char *path, int flags, ...)
{
    /* Pretend like we always fail */
    return -1;
}

extern "C" WEAK int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

extern "C" WEAK int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

extern "C" WEAK int _times(struct tms *buf)
{
    return -1;
}

extern "C" WEAK int _stat(char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

extern "C" WEAK int _link(char *oldf, char *newf)
{
    errno = EMLINK;
    return -1;
}

extern "C" WEAK int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

extern "C" WEAK int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

#ifndef OS_WRAP_MALLOC
#include "cmsis.h"
#include <sys/lock.h>
#include <stdlib.h>

/**
  * BES_THREAD_SAFE_STRATEGY
  * 1. disable interrupt
  * 2. using mutex
  * Note, now malloc lock/unlock still disable interrupt for efficiency
  */
#ifdef RTOS
#ifdef AOS_FS_ENABLE
#define BES_THREAD_SAFE_STRATEGY 2
#else
#define BES_THREAD_SAFE_STRATEGY 1
#endif
#else
#define BES_THREAD_SAFE_STRATEGY 0
#endif

#if BES_THREAD_SAFE_STRATEGY > 0
struct __lock_pri
{
    uint32_t flag;
    uint32_t counter;
};

struct __lock_pri __lock___malloc_recursive_mutex;

/**
  * @brief malloc lock
  * if not define this, it will go to slow __retarget_lock_acquire_recursive
  * @param reent The reentrance struct
  */
extern "C"
void __malloc_lock(struct _reent *reent)
{
    uint32_t flag;
    struct __lock_pri *_lock =  &__lock___malloc_recursive_mutex;
    flag = int_lock();
    if (_lock->counter == 0) {
        _lock->flag = flag;
    }
    _lock->counter++;
}

/**
  * @brief Release malloc lock
  * @param reent The reentrance struct
  */
extern "C"
void __malloc_unlock(struct _reent *reent)
{
    uint32_t flag = __lock___malloc_recursive_mutex.flag;
    struct __lock_pri *_lock =  &__lock___malloc_recursive_mutex;
    _lock->counter--;
    if (_lock->counter == 0) {
        int_unlock(flag);
    }
}
#endif

#if BES_THREAD_SAFE_STRATEGY == 1
struct __lock_pri  __lock___sinit_recursive_mutex;
struct __lock_pri  __lock___sfp_recursive_mutex;
struct __lock_pri  __lock___atexit_recursive_mutex;
struct __lock_pri  __lock___at_quick_exit_mutex;
struct __lock_pri  __lock___env_recursive_mutex;
struct __lock_pri  __lock___tz_mutex;
struct __lock_pri  __lock___dd_hash_mutex;
struct __lock_pri  __lock___arc4random_mutex;

extern "C"
void __retarget_lock_init(_LOCK_T *lock)
{
    struct __lock_pri *lock_pri;
    if (!lock) {
        return;
    }

    lock_pri = (struct __lock_pri *)malloc(sizeof(struct __lock_pri));
    if (lock_pri != NULL) {
        lock_pri->flag = 0;
        lock_pri->counter = 0;
        *lock = (_LOCK_T)lock_pri;
        return;
    }
}

extern "C"
void
__retarget_lock_init_recursive(_LOCK_T *lock)
{
    return __retarget_lock_init(lock);
}

extern "C"
void
__retarget_lock_close(_LOCK_T lock)
{
    free((void *)lock);
}

extern "C"
void
__retarget_lock_close_recursive(_LOCK_T lock)
{
    return __retarget_lock_close(lock);
}

extern "C"
void
__retarget_lock_acquire (_LOCK_T lock)
{
    struct __lock_pri *_lock = (struct __lock_pri *)lock;
    uint32_t flag = int_lock();
    if (_lock->counter == 0) {
        _lock->flag = flag;
    }
    _lock->counter++;
}

extern "C"
void
__retarget_lock_acquire_recursive (_LOCK_T lock)
{
    return __retarget_lock_acquire(lock);
}

extern "C"
int
__retarget_lock_try_acquire(_LOCK_T lock)
{
    __retarget_lock_acquire (lock);
    return 0;
}

extern "C"
int
__retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return __retarget_lock_try_acquire(lock);
}

extern "C"
void
__retarget_lock_release (_LOCK_T lock)
{
    struct __lock_pri *_lock = (struct __lock_pri *)lock;
    uint32_t flag = _lock->flag;

    _lock->counter--;
    if (_lock->counter == 0) {
        int_unlock(flag);
    }
}

extern "C"
void
__retarget_lock_release_recursive (_LOCK_T lock)
{
    return __retarget_lock_release (lock);
}

#elif BES_THREAD_SAFE_STRATEGY == 2

#include "cmsis_os.h"
struct bes_libc_lock
{
    osMutexId_t lock;
};

struct bes_libc_lock __lock___sinit_recursive_mutex;
struct bes_libc_lock __lock___sfp_recursive_mutex;
struct bes_libc_lock __lock___atexit_recursive_mutex;
struct bes_libc_lock __lock___at_quick_exit_mutex;
struct bes_libc_lock __lock___env_recursive_mutex;
struct bes_libc_lock __lock___tz_mutex;
struct bes_libc_lock __lock___dd_hash_mutex;
struct bes_libc_lock __lock___arc4random_mutex;

static int bes_libc_lock_init(void)
{
    osMutexAttr_t attr = { 0 };

    attr.attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust;

    __lock___sinit_recursive_mutex.lock  = osMutexNew(&attr);
    __lock___sfp_recursive_mutex.lock    = osMutexNew(&attr);
    __lock___atexit_recursive_mutex.lock = osMutexNew(&attr);
    __lock___at_quick_exit_mutex.lock    = osMutexNew(&attr);
    __lock___env_recursive_mutex.lock    = osMutexNew(&attr);
    __lock___tz_mutex.lock               = osMutexNew(&attr);
    __lock___dd_hash_mutex.lock          = osMutexNew(&attr);
    __lock___arc4random_mutex.lock       = osMutexNew(&attr);

    /*no need test the result */
    return 0;
}

int (*libc_lock_init)(void) __attribute__ ((section (".preinit_array"))) =  bes_libc_lock_init;

extern "C"
void __retarget_lock_init(_LOCK_T *lock)
{
    struct bes_libc_lock *_lock;
    osMutexAttr_t attr = { 0 };

     _lock = (struct bes_libc_lock *)malloc(sizeof(struct bes_libc_lock));

    attr.attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust;
    _lock->lock = osMutexNew(&attr);
    ASSERT(_lock->lock != NULL, "non-recursive lock allocation failed");
    *lock = (_LOCK_T)_lock;
}

extern "C"
void
__retarget_lock_init_recursive(_LOCK_T *lock)
{
    return __retarget_lock_init(lock);
}

extern "C"
void
__retarget_lock_close(_LOCK_T lock)
{
    if (lock == NULL)
        return;

    struct bes_libc_lock *_lock = (struct bes_libc_lock *)lock;
    osMutexId_t mutex = _lock->lock;
    if (mutex)
        osMutexDelete(mutex);
    free(lock);
}

extern "C"
void
__retarget_lock_close_recursive(_LOCK_T lock)
{
    return __retarget_lock_close(lock);
}

extern "C"
void
__retarget_lock_acquire (_LOCK_T lock)
{
    struct bes_libc_lock *_lock = (struct bes_libc_lock *)lock;

    if (lock == NULL)
        return;

    osMutexId_t mutex = _lock->lock;
    if (mutex == NULL)
        return;
    if (osKernelGetState() != osKernelRunning)
        return;
    osStatus_t status = osMutexAcquire(mutex, osWaitForever);
    if (status != osOK)
        ASSERT(0, "non-recursive lock failed");
    return;
}

extern "C"
void
__retarget_lock_acquire_recursive (_LOCK_T lock)
{
    return __retarget_lock_acquire(lock);
}

extern "C"
int
__retarget_lock_try_acquire(_LOCK_T lock)
{
    __retarget_lock_acquire (lock);
    return 0;
}

extern "C"
int
__retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return __retarget_lock_try_acquire(lock);
}

extern "C"
void
__retarget_lock_release (_LOCK_T lock)
{
    struct bes_libc_lock *_lock = (struct bes_libc_lock *)lock;

    if (lock == NULL)
        return;

    osMutexId_t mutex = _lock->lock;
    if (mutex == NULL)
        return;
    if (osKernelGetState() != osKernelRunning)
        return;
    osStatus_t status = osMutexRelease(mutex);
    if (status != osOK)
        ASSERT(0, "non-recursive release failed");
}

extern "C"
void
__retarget_lock_release_recursive (_LOCK_T lock)
{
    return __retarget_lock_release (lock);
}
#endif /*BES_THREAD_SAFE_STRATEGY */
#endif /*OS_WRAP_MALLOC*/

#endif /*LIBC_HOOKS*/
#endif // __GNUC__ && !__ARMCC_VERSION

