/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "os/os_specific.h"
#include "api/replay/replay_enums.h"

void Process::RegisterEnvironmentModification(const EnvironmentModification &modif)
{
}

void Process::ApplyEnvironmentModification()
{
}

uint32_t Process::LaunchProcess(const char *app, const char *workingDir, const char *cmdLine,
                                bool internal, ProcessResult *result)
{
  return 0;
}

uint32_t Process::LaunchScript(const char *script, const char *workingDir, const char *argList,
                               bool internal, ProcessResult *result)
{
  return 0;
}

rdcpair<ReplayStatus, uint32_t> Process::LaunchAndInjectIntoProcess(
    const char *app, const char *workingDir, const char *cmdLine,
    const rdcarray<EnvironmentModification> &envList, const char *capturefile,
    const CaptureOptions &opts, bool waitForExit)
{
  return {ReplayStatus::InternalError, 0};
}

bool Process::StartGlobalHook(const char *pathmatch, const char *logfile, const CaptureOptions &opts)
{
  return false;
}

bool Process::CanGlobalHook()
{
  return false;
}

bool Process::IsGlobalHookActive()
{
  return false;
}

void Process::StopGlobalHook()
{
}

bool Process::IsModuleLoaded(const char *module)
{
  return false;
}

void *Process::LoadModule(const char *module)
{
  return NULL;
}

void *Process::GetFunctionAddress(void *module, const char *function)
{
  return NULL;
}

uint32_t Process::GetCurrentPID()
{
  return (uint32_t)sceKernelGetProcessId();
}

uint64_t Process::GetMemoryUsage()
{
  return 0;
}

rdcpair<ReplayStatus, uint32_t> Process::InjectIntoProcess(uint32_t pid,
                                                           const rdcarray<EnvironmentModification> &env,
                                                           const char *capturefile,
                                                           const CaptureOptions &opts, bool waitForExit)
{
  return {ReplayStatus::InternalError, 0};
}

const char *Process::GetEnvVariable(const char *name)
{
  return NULL;
}

void Process::Shutdown()
{
}
