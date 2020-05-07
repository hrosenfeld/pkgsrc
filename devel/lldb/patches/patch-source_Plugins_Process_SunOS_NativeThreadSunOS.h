$NetBSD$

--- source/Plugins/Process/SunOS/NativeThreadSunOS.h.orig	2020-06-04 12:33:49.943140803 +0000
+++ source/Plugins/Process/SunOS/NativeThreadSunOS.h
@@ -0,0 +1,76 @@
+//===-- NativeThreadSunOS.h ---------------------------------- -*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef liblldb_NativeThreadSunOS_H_
+#define liblldb_NativeThreadSunOS_H_
+
+#include "lldb/Host/common/NativeThreadProtocol.h"
+
+#include <csignal>
+#include <map>
+#include <string>
+#include <libproc.h>
+
+namespace lldb_private {
+namespace process_sunos {
+
+class NativeProcessSunOS;
+
+class NativeThreadSunOS : public NativeThreadProtocol {
+  friend class NativeProcessSunOS;
+
+public:
+  NativeThreadSunOS(NativeProcessSunOS &process, lldb::tid_t tid);
+  ~NativeThreadSunOS();
+
+  // NativeThreadProtocol Interface
+  std::string GetName() override;
+
+  lldb::StateType GetState() override;
+
+  bool GetStopReason(ThreadStopInfo &stop_info,
+                     std::string &description) override;
+
+  NativeRegisterContext& GetRegisterContext() override;
+
+  Status SetWatchpoint(lldb::addr_t addr, size_t size, uint32_t watch_flags,
+                       bool hardware) override;
+
+  Status RemoveWatchpoint(lldb::addr_t addr) override;
+
+  Status SetHardwareBreakpoint(lldb::addr_t addr, size_t size) override;
+
+  Status RemoveHardwareBreakpoint(lldb::addr_t addr) override;
+
+  struct ps_lwphandle *GetLwpHdl() { return m_lwp; };
+
+private:
+  // Interface for friend classes
+
+  void SetStoppedBySignal(uint32_t signo, const siginfo_t *info = nullptr);
+  void SetStoppedByBreakpoint();
+  void SetStoppedByTrace();
+  void SetStoppedByExec();
+  void SetStoppedByWatchpoint(lldb::addr_t wpaddr, lldb::addr_t addr);
+  void SetStopped();
+  void SetRunning();
+  void SetStepping();
+
+  // Member Variables
+  lldb::StateType m_state;
+  ThreadStopInfo m_stop_info;
+  std::unique_ptr<NativeRegisterContext> m_reg_context_up;
+  std::string m_stop_description;
+  struct ps_lwphandle *m_lwp;
+};
+
+typedef std::shared_ptr<NativeThreadSunOS> NativeThreadSunOSSP;
+} // namespace process_sunos
+} // namespace lldb_private
+
+#endif // #ifndef liblldb_NativeThreadSunOS_H_
