$NetBSD$

--- source/Host/sunos/HostProcessSunOS.cpp.orig	2020-04-20 17:12:55.363891510 +0000
+++ source/Host/sunos/HostProcessSunOS.cpp
@@ -0,0 +1,126 @@
+//===-- HostProcessSunOS.cpp ------------------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#include "lldb/Host/Host.h"
+#include "lldb/Host/FileSystem.h"
+#include "lldb/Host/sunos/HostProcessSunOS.h"
+
+#include <libproc.h>
+
+using namespace lldb_private;
+
+namespace {
+        struct ps_prochandle *const kInvalidSunOSProcess = NULL;
+}
+
+HostProcessSunOS::HostProcessSunOS()
+    : HostNativeProcessBase(kInvalidSunOSProcess) {}
+
+HostProcessSunOS::HostProcessSunOS(lldb::process_t process)
+    : HostNativeProcessBase(process) {}
+
+HostProcessSunOS::HostProcessSunOS(::pid_t pid)
+    : HostNativeProcessBase(NULL) {
+  m_pid = pid;
+}
+
+HostProcessSunOS::~HostProcessSunOS() {}
+
+Status HostProcessSunOS::Signal(int signo) const {
+  if (m_process == kInvalidSunOSProcess) {
+    if (m_pid == 0) {
+      Status error;
+      error.SetErrorString("HostProcessSunOS refers to an invalid process");
+      return error;
+    } else {
+      return HostProcessSunOS::Signal(m_pid, signo);
+    }
+  }
+
+  return HostProcessSunOS::Signal(m_process, signo);
+}
+
+Status HostProcessSunOS::Signal(lldb::process_t process, int signo) {
+  const pstatus_t *ps = Pstatus(process);
+  
+  return HostProcessSunOS::Signal(ps->pr_pid, signo);
+}
+
+Status HostProcessSunOS::Signal(::pid_t pid, int signo) {
+  Status error;
+
+  if (-1 == ::kill(pid, signo))
+    error.SetErrorToErrno();
+
+  return error;
+}
+
+Status HostProcessSunOS::Terminate() { return Signal(SIGKILL); }
+
+Status HostProcessSunOS::GetMainModule(FileSpec &file_spec) const {
+  Status error;
+
+  char link_path[PATH_MAX];
+
+  if (m_process != kInvalidSunOSProcess) {
+    if (Pexecname(m_process, link_path, sizeof (link_path)) == NULL) {
+      error.SetErrorString("Unable to determine executable path");
+      return error;
+    }
+  } else {
+    // /proc/pid/object/a.out
+    char path[PATH_MAX];
+    ssize_t len;
+
+    if (snprintf(path, sizeof (path), "/proc/%" PRIu64 "/path/a.out", m_pid)
+        < 0) {
+      error.SetErrorString("Unable to build /proc/<pid>/path/a.out string");
+      return error;
+    }
+
+    error = FileSystem::Instance().Readlink(FileSpec(link_path), file_spec);
+    if (!error.Success())
+      return error;
+  }
+
+  return error;
+}
+
+lldb::pid_t HostProcessSunOS::GetProcessId() const {
+  if (m_process == kInvalidSunOSProcess)
+    return m_pid;
+
+  const pstatus_t *ps = Pstatus(m_process);
+
+  if (ps == NULL)
+    return 0;
+
+  return ps->pr_pid;
+}
+
+bool HostProcessSunOS::IsRunning() const {
+  if (m_process != kInvalidSunOSProcess) {
+    if (Pstate(m_process) == PS_RUN)
+      return true;
+    else
+      return false;
+  }
+
+  if (m_pid == 0)
+    return false;
+
+  // Send this process the null signal.  If it succeeds the process is running.
+  Status error = Signal(0);
+  return error.Success();
+}
+
+llvm::Expected<HostThread> HostProcessSunOS::StartMonitoring(
+    const Host::MonitorChildProcessCallback &callback, bool monitor_signals) {
+  return Host::StartMonitoringChildProcess(callback, GetProcessId(),
+                                           monitor_signals);
+}
