#include <psp2/kernel/clib.h>

#ifndef NDEBUG
#define LOG(...) \
do { \
	sceClibPrintf(__VA_ARGS__); \
} while (0)
#else
#define LOG(...)
#endif
