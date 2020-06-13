#include <vitasdkkern.h>
#include <taihen.h>

#include <inttypes.h>
#include <string.h>

#include "api.h"

static tai_hook_ref_t ksceKernelLaunchAppRef;
static tai_hook_ref_t ksceKernelStartPreloadedModulesRef;
static SceUID hook;

static SceUID ksceKernelLaunchAppPatched(char *titleid, uint32_t flags, char *path, void *unk) {

  SceAppMgrLaunchParam* param = (SceAppMgrLaunchParam*)unk;
  ksceDebugPrintf("titleid: %s, flags: %" PRIu32 ", path: %s, unk: %p\n", titleid, flags, path, unk);
  ksceDebugPrintf("param???: size: %u\n", param->size);
  for (size_t index = 0; index < param->size; ++index) {
      if ((index % 8) == 0) {
          ksceDebugPrintf("\n");
      }
      ksceDebugPrintf("0x%.2x ", (unsigned char)((char*)unk)[index] & 0xff);
  }
  ksceDebugPrintf("\n");
  return TAI_CONTINUE(int, ksceKernelLaunchAppRef, titleid, flags, path, unk); // returns pid
}

static int ksceKernelStartPreloadedModulesPatched(SceUID pid) {
  int res = TAI_CONTINUE(int, ksceKernelStartPreloadedModulesRef, pid);

  char titleid[32];
  ksceKernelGetProcessTitleId(pid, titleid, sizeof(titleid));
  ksceDebugPrintf("call preload for title: %s\n", titleid);

  if (strcmp(titleid, current_titleid) == 0) {
    ksceDebugPrintf("title: %s matched current title id\n", titleid);
    ksceKernelLoadStartModuleForPid(pid, "ur0:tai/gxm_inject.suprx", 0, NULL, 0, NULL, NULL);
  }

  return res;
}

/*static SceUID ksceKernelLaunchAppPatched(char *titleid, uint32_t flags, char *path, void *unk) {
  uintptr_t args[4];
  args[0] = (uintptr_t)titleid;
  args[1] = (uintptr_t)flags;
  args[2] = (uintptr_t)path;
  args[3] = (uintptr_t)unk;

  return ksceKernelRunWithStack(0x4000, _ksceKernelLaunchAppPatched, args);
}*/

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {

    //hook = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelLaunchAppRef, "SceProcessmgr", 0xEB1F8EF7, 0x68068618, ksceKernelLaunchAppPatched);
  hook = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelStartPreloadedModulesRef, "SceKernelModulemgr", 0xC445FA63, 0x432DCC7A, ksceKernelStartPreloadedModulesPatched);
  if (hook < 0) {
    hook = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelStartPreloadedModulesRef, "SceKernelModulemgr", 0x92C9FFC2, 0x998C7AE9, ksceKernelStartPreloadedModulesPatched);
  }

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {

    taiHookReleaseForKernel(hook, ksceKernelStartPreloadedModulesRef);

    return SCE_KERNEL_STOP_SUCCESS;
}