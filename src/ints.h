#ifndef __INTS__H___
#define __INTS__H___

#if !defined(HAVE_CONFIG)
#include "riack-config.h"
#endif

#if defined(HAVE_STDINT_H) && !defined(_WIN32)
#include <stdint.h>
#else

#ifndef int32_t
typedef signed __int32 int32_t;
#endif

#ifndef uint32_t
typedef unsigned __int32 uint32_t;
#endif

#ifndef int64_t
typedef signed __int64 int64_t;
#endif

#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#define INT32_MIN _I32_MIN
#define INT32_MAX _I32_MAX

#define UINT32_MIN _UI32_MIN
#define UINT32_MAX _UI32_MAX

#define INT64_MIN _I64_MIN
#define INT64_MAX _I64_MAX

#define UINT64_MIN _UI64_MIN
#define UINT64_MAX _UI64_MAX

#endif

#endif //__INTS__H___
