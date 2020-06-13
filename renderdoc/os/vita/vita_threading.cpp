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

#include "common/common.h"
#include "os/os_specific.h"

#include <time.h>
#include <unistd.h>

double Timing::GetTickFrequency()
{
  return 1000000.0;
}

uint64_t Timing::GetTick()
{
  return 0;
}

uint64_t Timing::GetUnixTimestamp()
{
  return (uint64_t)time(NULL);;
}

time_t Timing::GetUTCTime()
{
  return time(NULL);
}

namespace Atomic
{
int32_t Inc32(volatile int32_t *i)
{
  return __sync_add_and_fetch(i, int32_t(1));
}

int32_t Dec32(volatile int32_t *i)
{
  return __sync_add_and_fetch(i, int32_t(-1));
}

int64_t Inc64(volatile int64_t *i)
{
  return __sync_add_and_fetch(i, int64_t(1));
}

int64_t Dec64(volatile int64_t *i)
{
  return __sync_add_and_fetch(i, int64_t(-1));
}

int64_t ExchAdd64(volatile int64_t *i, int64_t a)
{
  return __sync_add_and_fetch(i, int64_t(a));
}

int32_t CmpExch32(volatile int32_t *dest, int32_t oldVal, int32_t newVal)
{
  return __sync_val_compare_and_swap(dest, oldVal, newVal);
}
};

namespace Threading
{
template <>
CriticalSection::CriticalSectionTemplate()
{
}

template <>
CriticalSection::~CriticalSectionTemplate()
{
}

template <>
void CriticalSection::Lock()
{
}

template <>
bool CriticalSection::Trylock()
{
  return false;
}

template <>
void CriticalSection::Unlock()
{
}

template <>
RWLock::RWLockTemplate()
{
}

template <>
RWLock::~RWLockTemplate()
{
}

template <>
void RWLock::WriteLock()
{
}

template <>
bool RWLock::TryWritelock()
{
  return false;
}

template <>
void RWLock::WriteUnlock()
{
}

template <>
void RWLock::ReadLock()
{
}

template <>
bool RWLock::TryReadlock()
{
  return false;
}

template <>
void RWLock::ReadUnlock()
{
}

struct ThreadInitData
{
  std::function<void()> entryFunc;
};

static int sThreadInit(SceSize args, void *init)
{
  ThreadInitData **data = (ThreadInitData **)init;

  // delete before going into entry function so lifetime is limited.
  ThreadInitData local = **data;
  delete *data;

  local.entryFunc();

  return 0;
}

void Init()
{
}

void Shutdown()
{
}

// allocate a TLS slot in our per-thread vectors with an atomic increment.
// Note this is going to be 1-indexed because Inc64 returns the post-increment
// value
uint64_t AllocateTLSSlot()
{
  return 0;
}

// look up our per-thread vector.
void *GetTLSValue(uint64_t slot)
{
  return NULL;
}

void SetTLSValue(uint64_t slot, void *value)
{
}

ThreadHandle CreateThread(std::function<void()> entryFunc)
{
  ThreadInitData *initData = new ThreadInitData();
  initData->entryFunc = entryFunc;

  SceUID thid = sceKernelCreateThread("thread", &sThreadInit, 0x40, 0x4000, 0, 0, NULL);
  if (thid < 0)
  {
    delete initData;
    return (ThreadHandle)0;
  }

  int res = sceKernelStartThread(thid, sizeof(initData), (void *)&initData);
  if (res < 0)
  {
    delete initData;
    sceKernelDeleteThread(thid);
    return (ThreadHandle)-1;
  }

  return (ThreadHandle)thid;
}

uint64_t GetCurrentID()
{
  return (uint64_t)sceKernelGetThreadId();
}

void JoinThread(ThreadHandle handle)
{
  if (handle == (ThreadHandle)-1)
    return;
  sceKernelWaitThreadEnd(handle, NULL, NULL);
}
void DetachThread(ThreadHandle handle)
{
  if (handle == (ThreadHandle)-1)
    return;
  sceKernelDeleteThread(handle);
}
void CloseThread(ThreadHandle handle)
{
  if (handle == (ThreadHandle)-1)
    return;
  sceKernelDeleteThread(handle);
}

void SetCurrentThreadName(rdcstr &threadName)
{
  RDCDEBUG("SetCurrentThreadName: %s", threadName.c_str());
}

void SetCurrentThreadName(const rdcstr &threadName)
{
  RDCDEBUG("SetCurrentThreadName: %s", threadName.c_str());
}

void KeepModuleAlive()
{
}

void ReleaseModuleExitThread()
{
}

void Sleep(uint32_t milliseconds)
{
  sceKernelDelayThread(milliseconds);
}
};