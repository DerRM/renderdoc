#include <psp2/kernel/clib.h>


#define LOG(...) \
	do { \
		sceClibPrintf(__VA_ARGS__); \
	} while (0)
