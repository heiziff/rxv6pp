/*! \file futex.h
 * \brief header for userspace mutexes
 */

#ifndef INCLUDED_user_futex_h
#define INCLUDED_user_futex_h

#ifdef __cplusplus
extern "C" {
#endif

//! the mutex type
typedef _Atomic unsigned long long osdev_mutex_t; // TODO: change

//! not technically needed, but heavily suggested
typedef unsigned long long osdev_mutex_underlying_t;

//! initialize mutex
void osdev_mutex_init(osdev_mutex_t *mutex);
//! lock mutex
void osdev_mutex_lock(osdev_mutex_t *mutex);
//! unlock mutex
void osdev_mutex_unlock(osdev_mutex_t *mutex);

//! try lock mutex
bool osdev_mutex_trylock(osdev_mutex_t *mutex);

#endif
