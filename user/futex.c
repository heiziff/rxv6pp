#include "user/futex.h"
#include <stdatomic.h>

void osdev_mutex_init(osdev_mutex_t *mutex) {
    mutex->inner = 0;
}

void osdev_mutex_lock(osdev_mutex_t *mutex) {
    osdev_mutex_t other = {0};
    while(!atomic_compare_exchange_weak(&mutex->inner, &other.inner, 1))
        other.inner = 0;
}
//! unlock mutex
void osdev_mutex_unlock(osdev_mutex_t *mutex) {
    atomic_store(&mutex->inner, 0);
}

//! try lock mutex
bool osdev_mutex_trylock(osdev_mutex_t *mutex) {
    osdev_mutex_lock(mutex);
    return true;
}
