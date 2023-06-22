/*! \file futex.h
 * \brief header for userspace mutexes
 */

#ifndef INCLUDED_user_futex_h
#define INCLUDED_user_futex_h

//! not technically needed, but heavily suggested
typedef unsigned long long osdev_mutex_underlying_t;

#ifndef __cplusplus

#include <stdbool.h>
#include <kernel/sys_futex.h>
#include "user.h"

typedef _Atomic osdev_mutex_underlying_t osdev_mutex_inner_t;

#else

#include <atomic>

using osdev_mutex_inner_t = std::atomic<osdev_mutex_underlying_t>;

#endif

#ifdef __cplusplus
extern "C" {
#endif

struct osdev_mutex_t;
//! having fun with linkage...
typedef struct osdev_mutex_t osdev_mutex_t;

/*!
 * \brief wrap object to (hopefully) be ABI compatible
 */
struct osdev_mutex_t {
  //! an atomic counter
  osdev_mutex_inner_t inner;

  //! \attention you can add more stuff here if you want
};

//! initialize mutex
void osdev_mutex_init(osdev_mutex_t *mutex);
//! lock mutex
void osdev_mutex_lock(osdev_mutex_t *mutex);
//! unlock mutex
void osdev_mutex_unlock(osdev_mutex_t *mutex);

//! try lock mutex
bool osdev_mutex_trylock(osdev_mutex_t *mutex);

#ifdef __cplusplus
}
#endif


#endif
