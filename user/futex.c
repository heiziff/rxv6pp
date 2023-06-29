#include "user/futex.h"
#include <stdatomic.h>

void osdev_mutex_init(osdev_mutex_t *mutex) {
  mutex->inner = 0;
  printf("before\n");
  futex(&mutex->inner, FUTEX_INIT, 0);
}

void osdev_mutex_lock(osdev_mutex_t *mutex) {
  osdev_mutex_t other = {0};
  while (!atomic_compare_exchange_weak(&mutex->inner, &other.inner, 1)) {
    int res = futex(&mutex->inner, FUTEX_WAIT, 1);
    printf("futex res %d\n", res);
    if (res == 0) {
      printf("pid x 1USRWRAPPER locked mutex: %d\n", mutex->inner);
      return;
    }
    other.inner = 0;
  }
  printf("2USRWRAPPER pid %d locked mutex: %d\n", getpid(), mutex->inner);
  // while (!atomic_compare_exchange_weak(&mutex->inner, &other.inner, 1)) other.inner = 0;
}
//! unlock mutex
void osdev_mutex_unlock(osdev_mutex_t *mutex) {
  printf("trying to unlock futex: %d, ", mutex->inner);
  printf("after futex: %d\n", mutex->inner);
  uint64 wakeups = futex(&mutex->inner, FUTEX_WAKE, 1);
  if (!wakeups) atomic_store(&mutex->inner, 0);
  printf("%d wakeups\n", wakeups);
}

//! try lock mutex
bool osdev_mutex_trylock(osdev_mutex_t *mutex) {
  osdev_mutex_t other = {0};
  return atomic_compare_exchange_weak(&mutex->inner, &other.inner, 1);
  // osdev_mutex_lock(mutex);
  // return true;
}
