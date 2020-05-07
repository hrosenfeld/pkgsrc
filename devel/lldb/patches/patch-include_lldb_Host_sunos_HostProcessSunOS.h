$NetBSD$

--- include/lldb/Host/sunos/HostProcessSunOS.h.orig	2020-04-20 13:08:49.497991524 +0000
+++ include/lldb/Host/sunos/HostProcessSunOS.h
@@ -0,0 +1,47 @@
+//===-- HostProcessSunOS.h --------------------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef lldb_Host_HostProcesSunOS_h_
+#define lldb_Host_HostProcesSunOS_h_
+
+#include "lldb/Host/HostNativeProcessBase.h"
+#include "lldb/Utility/Status.h"
+#include "lldb/lldb-types.h"
+
+namespace lldb_private {
+
+class FileSpec;
+
+class HostProcessSunOS : public HostNativeProcessBase {
+public:
+  HostProcessSunOS();
+  HostProcessSunOS(lldb::process_t process);
+  HostProcessSunOS(::pid_t pid);
+  ~HostProcessSunOS() override;
+
+  virtual Status Signal(int signo) const;
+  static Status Signal(lldb::process_t process, int signo);
+  static Status Signal(::pid_t pid, int signo);
+
+  Status Terminate() override;
+  Status GetMainModule(FileSpec &file_spec) const override;
+
+  struct ps_prochandle *GetProcHdl() const { return m_process; };
+  lldb::pid_t GetProcessId() const override;
+  bool IsRunning() const override;
+
+  llvm::Expected<HostThread>
+  StartMonitoring(const Host::MonitorChildProcessCallback &callback,
+                  bool monitor_signals) override;
+private:
+  ::pid_t m_pid;
+};
+
+} // namespace lldb_private
+
+#endif // lldb_Host_HostProcesSunOS_h_
