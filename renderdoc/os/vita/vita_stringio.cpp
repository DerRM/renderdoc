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

#include "common/formatting.h"
#include "os/os_specific.h"

namespace Keyboard
{
void Init()
{
}

bool PlatformHasKeyInput()
{
  return false;
}

void AddInputWindow(WindowingSystem windowSystem, void *wnd)
{
}

void RemoveInputWindow(WindowingSystem windowSystem, void *wnd)
{
}

bool GetKeyState(int key)
{
  return false;
}
}

namespace FileIO
{
// in posix/.../..._stringio.cpp
rdcstr GetTempRootPath()
{
  return "ux0:temp";
}

rdcstr GetHomeFolderFilename()
{
  return "";
}

rdcstr GetTempFolderFilename()
{
  return "";
}

void GetExecutableFilename(rdcstr &filename)
{
}

void GetLibraryFilename(rdcstr &filename)
{
}

rdcstr GetAppFolderFilename(const rdcstr &filename)
{
  return "";
}

void CreateParentDirectory(const rdcstr &filename)
{
}

bool IsRelativePath(const rdcstr &path)
{
    return false;
}

rdcstr GetFullPathname(const rdcstr &filename)
{
  return "";
}

rdcstr FindFileInPath(const rdcstr &fileName)
{
  return "";
}

rdcstr GetReplayAppFilename()
{
  return "";
}

void GetDefaultFiles(const char *logBaseName, rdcstr &capture_filename, rdcstr &logging_filename,
                     rdcstr &target)
{
  const char *mod = "renderdoc_vita";

  target = rdcstr(mod);

  time_t t = time(NULL);
  tm now = *localtime(&t);

  char temp_folder[2048] = {0};
  strcpy(temp_folder, GetTempRootPath().c_str());

  capture_filename =
    StringFormat::Fmt("%s/RenderDoc/%s_%04d.%02d.%02d_%02d.%02d.rdc", temp_folder, mod,
                        1900 + now.tm_year, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min);

  // set by UI when launching programs so all logging goes to the same file
  char *logfile_override = getenv("RENDERDOC_DEBUG_LOG_FILE");
  if(logfile_override)
    logging_filename = rdcstr(logfile_override);
  else
    logging_filename = StringFormat::Fmt(
        "%s/RenderDoc/%s_%04d.%02d.%02d_%02d.%02d.%02d.log", temp_folder, logBaseName,
        1900 + now.tm_year, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
}

uint64_t GetModifiedTimestamp(const rdcstr &filename)
{
  return 0;
}

uint64_t GetFileSize(const rdcstr &filename)
{
  return 0;
}

bool Copy(const char *from, const char *to, bool allowOverwrite)
{
  return false;
}

bool Move(const char *from, const char *to, bool allowOverwrite)
{
  return false;
}

void Delete(const char *path)
{
}

void GetFilesInDirectory(const char *path, rdcarray<PathEntry> &ret)
{
}

FILE *fopen(const char *filename, const char *mode)
{
  return NULL;
}

rdcstr ErrorString()
{
  return "";
}

size_t fread(void *buf, size_t elementSize, size_t count, FILE *f)
{
  return 0;
}
size_t fwrite(const void *buf, size_t elementSize, size_t count, FILE *f)
{
  return 0;
}

uint64_t ftell64(FILE *f)
{
  return 0;
}
void fseek64(FILE *f, uint64_t offset, int origin)
{
}

bool feof(FILE *f)
{
  return false;
}

void ftruncateat(FILE *f, uint64_t length)
{
}

bool fflush(FILE *f)
{
  return false;
}

int fclose(FILE *f)
{
  return false;
}

bool exists(const char *filename)
{
  return false;
}

void ReleaseFDAfterFork()
{
}

rdcstr logfile_readall(const char *filename)
{
  return "";
}

LogFileHandle *logfile_open(const char *filename)
{
  return NULL;
}

void logfile_append(LogFileHandle *logHandle, const char *msg, size_t length)
{
}

void logfile_close(LogFileHandle *logHandle, const char *deleteFilename)
{
}
};

namespace StringFormat
{
rdcstr Wide2UTF8(const rdcwstr &s)
{
  return "";
}

rdcwstr UTF82Wide(const rdcstr &s)
{
  return L"";
}

rdcstr sntimef(time_t utcTime, const char *format)
{
  tm *tmv = localtime(&utcTime);

  // conservatively assume that most formatters will replace like-for-like (e.g. %H with 12) and
  // a few will increase (%Y to 2019) but generally the string will stay the same size.
  size_t len = strlen(format) + 16;

  size_t ret = 0;
  char *buf = NULL;

  // loop until we have successfully formatted
  while(ret == 0)
  {
    // delete any previous buffer
    delete[] buf;

    // alloate new one of the new size
    buf = new char[len + 1];
    buf[len] = 0;

    // try formatting
    ret = strftime(buf, len, format, tmv);

    // double the length for next time, if this failed
    len *= 2;
  }

  rdcstr str = buf;

  // delete successful buffer
  delete[] buf;

  return str;
}

void Shutdown()
{
}
};

namespace OSUtility
{
void WriteOutput(int channel, const char *str)
{
  if(channel == OSUtility::Output_DebugMon)
    sceClibPrintf(str);
  else if(channel == OSUtility::Output_StdOut)
    fprintf(stdout, "%s", str);
  else if(channel == OSUtility::Output_StdErr)
    fprintf(stderr, "%s", str);
}

uint64_t GetMachineIdent()
{
  uint64_t ret = MachineIdent_PSVita;
  ret |= MachineIdent_Arch_ARM;
  ret |= MachineIdent_32bit;
  ret |= MachineIdent_GPU_IMG;
  return ret;
}
};

