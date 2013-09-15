#ifndef __INTS__H___
#define __INTS__H___

#if !defined(HAVE_CONFIG)
#include "riack-config.h"
#endif

#if defined(HAVE_STDINT_H) && !defined(_WIN32)
#include <stdint.h>
#else
#define int32_t      signed __int32
#define INT32_MIN    _I32_MIN
#define INT32_MAX    _I32_MAX
#define uint32_t     unsigned __int32
#define UINT32_MIN   _UI32_MIN
#define UINT32_MAX   _UI32_MAX
#define int64_t      signed __int64
#define INT64_MIN    _I64_MIN
#define INT64_MAX    _I64_MAX
#define uint64_t     unsigned __int64
#define UINT64_MIN   _UI64_MIN
#define UINT64_MAX   _UI64_MAX
#define uint8_t      unsigned char
#endif

#endif //__INTS__H___
