/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2019 Baldur Karlsson
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

#pragma once

class RemoteHost;

// do not include any headers here, they must all be in QRDInterface.h
#include "QRDInterface.h"

class PersistantConfig;
class ReplayManager;

struct RemoteHostData;

// to enable easy copying around of these hosts as well as allowing graceful behaviour when hosts
// are unexpectedly removed (such as disconnecting an auto-populated device) these structs are
// copied around and they have a shared locked data pointer. All accessors then lock and look up the
// data there to fetch or modify
DOCUMENT("A handle for interacting with a remote server on a given host.");
class RemoteHost
{
public:
  DOCUMENT("");
  RemoteHost();
  RemoteHost(const rdcstr &host);
  RemoteHost(const RemoteHost &o);
#if !defined(SWIG)
  RemoteHost &operator=(const RemoteHost &o);
#endif
  ~RemoteHost();
  VARIANT_CAST(RemoteHost);

  DOCUMENT("");
  bool operator==(const RemoteHost &o) const { return m_hostname == o.m_hostname; }
  bool operator!=(const RemoteHost &o) const { return !(*this == o); }
  bool operator<(const RemoteHost &o) const { return m_hostname < o.m_hostname; }
  DOCUMENT(
      "Ping the host to check current status - if the server is running, connection status, etc.");
  void CheckStatus();
  DOCUMENT(
      "Runs the command specified in :data:`runCommand`. Returns :class:`ReplayStatus` which "
      "indicates success or the type of failure.");
  ReplayStatus Launch();

  DOCUMENT("``True`` if a remote server is currently running on this host.");
  bool IsServerRunning() const;
  DOCUMENT("``True`` if an active connection exists to this remote server.");
  bool IsConnected() const;
  DOCUMENT("``True`` if someone else is currently connected to this server.");
  bool IsBusy() const;
  DOCUMENT("``True`` if there is a code version mismatch with this server.");
  bool IsVersionMismatch() const;

  DOCUMENT("The hostname of this host.");
  const rdcstr &Hostname() const { return m_hostname; }
  DOCUMENT("The friendly name for this host, if available (if empty, the Hostname is used).");
  rdcstr FriendlyName() const;
  DOCUMENT("The command to run locally to try to launch the server remotely.");
  rdcstr RunCommand() const;
  DOCUMENT(R"(Sets the run command. See :meth:`RunCommand`.

:param str cmd: The new command to set.
)");
  void SetRunCommand(const rdcstr &cmd);

  DOCUMENT("The last folder browsed to on this host, to provide a reasonable default path.");
  rdcstr LastCapturePath() const;
  DOCUMENT(R"(Sets the last folder browsed to. See :meth:`LastCapturePath`.

:param str path: The new path to set.
)");
  void SetLastCapturePath(const rdcstr &path);

  DOCUMENT(
      "The :class:`DeviceProtocolController` for this host, or ``None`` if no protocol is in use");
  IDeviceProtocolController *Protocol() const { return m_protocol; }
  DOCUMENT(R"(
Returns the name to display for this host in the UI, either :meth:`FriendlyName` if it is valid, or
:meth:`Hostname` if not.
)");
  rdcstr Name() const
  {
    rdcstr friendlyName = FriendlyName();
    return !friendlyName.isEmpty() ? friendlyName : m_hostname;
  }
  DOCUMENT("Returns ``True`` if this host represents the special localhost device.");
  bool IsLocalhost() const { return m_hostname == "localhost"; }
  DOCUMENT("Returns ``True`` if this host represents a valid remote host.");
  bool IsValid() const { return m_data && !m_hostname.isEmpty(); }
private:
  // this is immutable and is used as a key to look up data, it's always valid as RemoteHost objects
  // are created with it
  rdcstr m_hostname;

  IDeviceProtocolController *m_protocol = NULL;

  // self-deleting shared and locked data store
  RemoteHostData *m_data = NULL;

  // allow config to set our data
  friend class PersistantConfig;
  void SetFriendlyName(const rdcstr &name);

  // allow ReplayManager to call these functions to change the status. Otherwise they are read-only
  // except by calling CheckStatus()
  friend class ReplayManager;
  void SetConnected(bool connected);
  void SetShutdown();

  void UpdateStatus(ReplayStatus status);
};

DECLARE_REFLECTION_STRUCT(RemoteHost);
