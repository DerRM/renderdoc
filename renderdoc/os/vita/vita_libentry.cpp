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

#include "core/core.h"
#include "hooks/hooks.h"
#include "os/os_specific.h"
#include "vita_specific.h"

int _newlib_heap_size_user = 32 * 1024 * 1024;

//void* __dso_handle = (void*) &__dso_handle;

//extern "C" void _init_vita_reent( void );
//extern "C" void _free_vita_reent( void );

//extern void __libc_init_array( void );
//extern void __libc_fini_array( void );

//extern "C" void _init_vita_newlib( void ) 
//{
//	_init_vita_reent( );
//}

//extern "C" void _free_vita_newlib( void )
//{
//	_free_vita_reent( );
//}

//extern "C" void _fini( void ) { }
//extern "C" void _init( void ) { }

// DllMain equivalent
void library_loaded()
{
  if(LibraryHooks::Detect("renderdoc__replay__marker"))
  {
    RDCDEBUG("Not creating hooks - in replay app");

    RenderDoc::Inst().SetReplayApp(true);

    RenderDoc::Inst().Initialise();

    return;
  }
  else
  {
    RenderDoc::Inst().Initialise();

    const char *capturefile = Process::GetEnvVariable("RENDERDOC_CAPFILE");
    const char *opts = Process::GetEnvVariable("RENDERDOC_CAPOPTS");

    if(opts)
    {
      CaptureOptions optstruct;
      optstruct.DecodeFromString(opts);

      RDCLOG("Using delay for debugger %u", optstruct.delayForDebugger);

      RenderDoc::Inst().SetCaptureOptions(optstruct);
    }

    if(capturefile)
    {
      RenderDoc::Inst().SetCaptureFileTemplate(capturefile);
    }

    rdcstr curfile;
    FileIO::GetExecutableFilename(curfile);

    RDCLOG("Loading into %s", curfile.c_str());

    LibraryHooks::RegisterHooks();

    // we have a short sleep here to allow target control to connect, since unlike windows we can't
    // suspend the process during startup.
    Threading::Sleep(15);
  }
}

// wrap in a struct to enforce ordering. This file is
// linked last, so all other global struct constructors
// should run first
struct init
{
  init() 
  { 
    library_loaded(); 
  }
} do_init;

extern "C" int _start(SceSize argc, const void *args) __attribute__ ((weak, alias ("module_start")));

extern "C" int module_start(SceSize argc, const void *args)
{
    return SCE_KERNEL_START_SUCCESS;
}

extern "C" int module_stop(SceSize argc, const void *args)
{
    return SCE_KERNEL_STOP_SUCCESS;
}

int main(int argc, char *argv[])
{
  while(true) {
    sceKernelDelayThread(1000);
  }
  return 0;
}