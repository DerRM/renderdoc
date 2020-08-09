#include <psp2/kernel/clib.h>

#ifndef NDEBUG
#define LOG(...) \
do { \
	sceClibPrintf(__VA_ARGS__); \
} while (0)
#else
#define LOG(...)
#endif

#ifndef NDEBUG
#define LOGD(...) \
do { \
	sceClibPrintf(__VA_ARGS__); \
} while (0)
#else
#define LOGD(...)
#endif