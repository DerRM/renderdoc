#ifndef __VITAHOOK_KERNEL_API_H__
#define __VITAHOOK_KERNEL_API_H__

#include <psp2kern/types.h>
#include <psp2kern/appmgr.h>

extern char current_titleid[32];

SceUID vitaHookStartApp(char* titleId, uint32_t flags, char* path, uint32_t unk);
void vitaHookSetCurrentTitle(char* titleId, uint32_t size);
void vitaHookSaveFile(const char* path, uint8_t* data, uint32_t size);

#endif
