#include <vitasdkkern.h>
#include <taihen.h>

#include <inttypes.h>
#include <string.h>

#include "api.h"

static tai_hook_ref_t ksceKernelStartPreloadedModulesRef;
static SceUID hook;

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

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {

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