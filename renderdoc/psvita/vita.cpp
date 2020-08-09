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

#include "vita.h"
#include "common/threading.h"
#include "core/core.h"
#include "core/remote_server.h"
#include "replay/replay_driver.h"
#include "serialise/serialiser.h"
#include "serialise/streamio.h"

namespace PSVita
{
}

struct VitaRemoteServer : public RemoteServer
{
  VitaRemoteServer(Network::Socket *socket, const rdcstr &deviceId) : RemoteServer(socket, deviceId)
  {
  }

  virtual ~VitaRemoteServer() override {}

  virtual void ShutdownConnection() override { RemoteServer::ShutdownConnection(); }

  virtual void ShutdownServerAndConnection() override
  {
    RemoteServer::ShutdownServerAndConnection();
  }

  virtual bool Ping() override
  {
    if(!Connected())
      return false;

    return RemoteServer::Ping();
  }

  virtual rdcpair<ReplayStatus, IReplayController *> OpenCapture(
      uint32_t proxyid, const char *filename, const ReplayOptions &opts,
      RENDERDOC_ProgressCallback progress) override
  {
    return RemoteServer::OpenCapture(proxyid, filename, opts, progress);
  }

  virtual rdcstr GetHomeFolder() override { return ""; }
  virtual rdcarray<PathEntry> ListFolder(const char *path) override
  {
    return RemoteServer::ListFolder(path);
  }

  virtual ExecuteResult ExecuteAndInject(const char *a, const char *w, const char *c,
                                         const rdcarray<EnvironmentModification> &env,
                                         const CaptureOptions &opts) override;
};

struct VitaController : public IDeviceProtocolHandler
{
  void Start()
  {
    if(running == 0)
    {
      Atomic::Inc32(&running);

      socket = Network::CreateUDPSocket("239.255.255.250", 1900);
      thread = Threading::CreateThread([]() { m_Inst.ThreadEntry(); });
      RenderDoc::Inst().RegisterShutdownFunction([]() { m_Inst.Shutdown(); });
    }
  }

  void Shutdown() {}

  struct Command
  {
    std::function<void()> meth;
    int32_t done = 0;
  };

  rdcarray<Command *> cmdqueue;

  void ThreadEntry()
  {
    Threading::SetCurrentThreadName("VitaController");

    while(Atomic::CmpExch32(&running, 1, 1) == 1)
    {
      uint8_t bytes[256] = {0};
      rdcstr addr;
      uint32_t length = 256;
      socket->RecvFrom(bytes, length, addr);

      if(socket->Connected())
      {
        rdcstr str((const char *const)bytes, length);

        if(str == "Hello World")
        {
          lock.Lock();

          if(devices.contains(addr))
          {
            continue;
          }

          devices.push_back(addr);

          lock.Unlock();
        }

        continue;
      }
      else
      {
        break;
      }
      /*  Command *cmd = NULL;

        {
          SCOPED_LOCK(lock);
          if(cmdqueue.empty())
            continue;

          cmd = cmdqueue[0];
          cmdqueue.erase(0);
        }

        cmd->meth();

        Atomic::Inc32(&cmd->done);*/
    }
  }

  void Invoke(std::function<void()> method)
  {
    Command cmd;
    cmd.meth = method;

    {
      SCOPED_LOCK(lock);
      cmdqueue.push_back(&cmd);
    }

    while(Atomic::CmpExch32(&cmd.done, 0, 0) == 0)
      Threading::Sleep(5);
  }

  rdcstr GetProtocolName() override { return "vita"; }
  rdcarray<rdcstr> GetDevices() override { return devices; }

  rdcstr GetFriendlyName(const rdcstr &URL) override
  {
    rdcstr ret;
    return ret;
  }

  bool SupportsMultiplePrograms(const rdcstr &URL) override { return false; }
  bool IsSupported(const rdcstr &URL) override
  {
    bool ret = true;
    return ret;
  }

  ReplayStatus StartRemoteServer(const rdcstr &URL) override
  {
    ReplayStatus status = ReplayStatus::Succeeded;
    return status;
  }

  rdcstr RemapHostname(const rdcstr &deviceID) override { return deviceID; }

  uint16_t RemapPort(const rdcstr &deviceID, uint16_t srcPort) override
  {
    if(srcPort >= RenderDoc_FirstTargetControlPort && srcPort <= RenderDoc_LastTargetControlPort)
      return srcPort;

    if(srcPort == RenderDoc_RemoteServerPort)
      return srcPort;

    uint16_t portbase = 31245;
    return portbase + srcPort;
  }

  IRemoteServer *CreateRemoteServer(Network::Socket *sock, const rdcstr &deviceID) override
  {
    return new VitaRemoteServer(sock, deviceID);
  }

  int32_t running = 0;

  rdcarray<rdcstr> devices;
  Threading::CriticalSection lock;
  Network::Socket *socket = nullptr;
  Threading::ThreadHandle thread;
  static VitaController m_Inst;

  static IDeviceProtocolHandler *Get()
  {
    m_Inst.Start();
    return &m_Inst;
  }
};

ExecuteResult VitaRemoteServer::ExecuteAndInject(const char *a, const char *w, const char *c,
                                                 const rdcarray<EnvironmentModification> &env,
                                                 const CaptureOptions &opts)
{
  ExecuteResult ret;

  rdcstr executable = a;
  if(executable.length() > 0)
  {
    executable = executable.substr(executable.length() - 10, 9);
  }

  WriteSerialiser ser(new StreamWriter(m_Socket, Ownership::Nothing), Ownership::Stream);
  ser.SetStreamingMode(true);

  {
    SCOPED_SERIALISE_CHUNK(20);
    SERIALISE_ELEMENT(executable);
  }

  Threading::Sleep(5 * 1000);
  ret.status = ReplayStatus::Succeeded;
  ret.ident = 38920;

  return ret;
}

VitaController VitaController::m_Inst;

DeviceProtocolRegistration vitaProtocol("vita", &VitaController::Get);
