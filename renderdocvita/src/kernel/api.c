#include "api.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <vitasdkkern.h>
#include <psp2/gxm.h>
#include <taihen.h>

#define APP_DIR "ux0:app"

char current_titleid[32] = {0};

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);
SceUID (*sceKernelLaunchApp)(char *titleid, uint32_t flags, char *path, void* unk);
SceUID (*sceKernelResumeProcess)(SceUID pid);
/*
static SceUID launchApp(void* args)
{
    char *titleId  = (char *)((uintptr_t *)args)[0];
    uint32_t flags = (uint32_t)((uintptr_t *)args)[1];
    char *path     = (char *)((uintptr_t *)args)[2];
    void *unk      = (void *)((uintptr_t *)args)[3];

    
    return pid;
}
*/

static SceUID hook;
static tai_hook_ref_t ref;

int sceGxmDraw_hooked(SceGxmContext* context, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void* indexData, unsigned int indexCount) {
    ksceDebugPrintf("hello from sceGxmDraw(context: %p, primType: %d, indexType: %d, indexData: %p, indexCount: %d) ;)\n", context, primType, indexType, indexData, indexCount);
    return TAI_CONTINUE(int, ref, context, primType, indexType, indexData, indexCount);
}

void vitaHookSetCurrentTitle(char* titleId, uint32_t size)
{
    int state = 0;
    ENTER_SYSCALL(state);

    int res = ksceKernelStrncpyUserToKernel(current_titleid, (uintptr_t)(titleId), size);
    if (res < 0)
    {
        EXIT_SYSCALL(state);
        ksceDebugPrintf("could not copy titleId to kernel space\n");
    }

    EXIT_SYSCALL(state);
}

void vitaHookInstallHooks(SceUID pid)
{
    hook = taiHookFunctionImportForKernel(pid, &ref, "SceGxm", 0x0D0AA0CB, 0xBC059AFC, &sceGxmDraw_hooked);
    if (hook < 0) 
    {
        ksceDebugPrintf("hook of sceGxmDraw failed with: %" PRIi32 "\n", hook);
    }
}

SceUID vitaHookStartApp(char* titleId, uint32_t flags, char* path, uint32_t unk){

    const size_t maxStringLength = 16;
    char titId[maxStringLength];
    const size_t execPathLength = 128;
    char execPath[execPathLength];

    int state = 0;
    ENTER_SYSCALL(state);
    
    int res = ksceKernelStrncpyUserToKernel(titId, (uintptr_t)(titleId), maxStringLength);
    res = ksceKernelStrncpyUserToKernel(execPath, (uintptr_t)(path), execPathLength);

    ksceDebugPrintf("execute %s in kernel mode at path %s with flags: %" PRIu32 "\n", titId, execPath, flags);

    if (res < 0)
    {
        EXIT_SYSCALL(state);
        return res;
    }

    int ret = module_get_export_func(KERNEL_PID, "SceProcessmgr", 0xEB1F8EF7, 0x68068618, (uintptr_t*)&sceKernelLaunchApp);
    ksceDebugPrintf("ret: %" PRIi32 "\n", ret);

    //ret = module_get_export_func(KERNEL_PID, "SceProcessmgr", 0xEB1F8EF7, 0x68068618, (uintptr_t*)&sceKernelResumeProcess);
    //ksceDebugPrintf("ret: %" PRIi32 "\n", ret);
    
    SceUID pid = 0;

    uint8_t bytes[64] = {
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0xb1, 0x14, 0x01, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0xa0, 0x31, 0x5a, 0x02, 0x40, 0x00, 0x00, 0x00, 
        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
    };

    //uint8_t kernel_bytes[64];
    //res = ksceKernelStrncpyUserToKernel(kernel_bytes, (uintptr_t)(bytes), 64);

    SceAppMgrLaunchParam param = {};
    param.size = sizeof(SceAppMgrLaunchParam);

    pid = ksceAppMgrLaunchAppByPath(execPath, NULL, 0, 0, &param, NULL);
    //ksceKernelResumeProcess(pid);
    //pid = sceKernelLaunchApp(titId, flags, execPath, bytes);
    
    int status = 0;
    ksceKernelGetProcessStatus(pid, &status);
    ksceDebugPrintf("process status %" PRIi32 "\n", status);

    ksceDebugPrintf("pid: %" PRIi32 " %" PRIu32 "\n", pid, (uint32_t)pid);

    EXIT_SYSCALL(state);
    return pid;
}
