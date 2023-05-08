
/*! \file user.h
 * \brief header for userspace standard library
 */

#ifndef INCLUDED_rt_test_assert_h
#define INCLUDED_rt_test_assert_h

#ifdef __cplusplus
extern "C" {
#endif

#include "user/user.h"

#define assert(assertion) \
	do { \
		if (!(assertion)) { \
			printf("assertion [" #assertion "] failed on [%s@" __FILE__ ":%d]\n", __func__, __LINE__); \
			exit(1); \
		} \
	} while(false)


#ifdef __cplusplus
}
#endif

#endif