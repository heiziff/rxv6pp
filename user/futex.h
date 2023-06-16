/*! \file futex.h
 * \brief header for userspace mutexes
 */

#ifndef INCLUDED_user_futex_h
#define INCLUDED_user_futex_h

#ifdef __cplusplus
extern "C" {
#endif

//! not technically needed, but heavily suggested
typedef unsigned long long osdev_mutex_underlying_t;

#ifndef __cplusplus

#include <stdbool.h>

typedef _Atomic osdev_mutex_underlying_t osdev_mutex_t;

#else

//! \brief C++ struct for wrapping
//! \attention disabled some C++ features to  lead to somewhat sane behavior
//! \attention never instantiate this class directly, only ever via a pointer or to instantiate
//! \attention only here for alignment/size purposes, ignore that
struct osdev_mutex_t {
    osdev_mutex_underlying_t inner;
    osdev_mutex_t() = default;
    osdev_mutex_t(const osdev_mutex_t&) = delete;
    osdev_mutex_t(osdev_mutex_t&&) = delete;
    osdev_mutex_t& operator=(const osdev_mutex_t&) = delete;
    osdev_mutex_t& operator=(osdev_mutex_t&&) = delete;
};

#endif

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
