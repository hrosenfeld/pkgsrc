$NetBSD$

--- source/Plugins/Process/SunOS/NativeRegisterContextSunOS.cpp.orig	2020-07-02 14:13:00.074731574 +0000
+++ source/Plugins/Process/SunOS/NativeRegisterContextSunOS.cpp
@@ -0,0 +1,77 @@
+//===-- NativeRegisterContextSunOS.cpp -------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#include "NativeRegisterContextSunOS.h"
+
+#include "lldb/Host/common/NativeProcessProtocol.h"
+
+using namespace lldb_private;
+using namespace lldb_private::process_sunos;
+
+// clang-format off
+#include <sys/types.h>
+#include <libproc.h>
+// clang-format on
+
+NativeRegisterContextSunOS::NativeRegisterContextSunOS(
+    NativeThreadProtocol &native_thread,
+    RegisterInfoInterface *reg_info_interface_p)
+    : NativeRegisterContextRegisterInfo(native_thread,
+                                        reg_info_interface_p) {}
+
+Status NativeRegisterContextSunOS::GetGRegSet(prgregset_t buf) {
+  if (Plwp_getregs(GetProcHdl(), m_thread.GetID(), buf) != 0) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  return Status();
+}
+
+Status NativeRegisterContextSunOS::GetFPRegSet(prfpregset_t *buf) {
+  if (Plwp_getfpregs(GetProcHdl(), m_thread.GetID(), buf) != 0) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  return Status();
+}
+
+Status NativeRegisterContextSunOS::SetGRegSet(prgregset_t buf) {
+  if (Plwp_setregs(GetProcHdl(), m_thread.GetID(), buf) != 0) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  return Status();
+}
+
+Status NativeRegisterContextSunOS::SetFPRegSet(prfpregset_t *buf) {
+  if (Plwp_setfpregs(GetProcHdl(), m_thread.GetID(), buf) != 0) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  return Status();
+}
+
+NativeProcessSunOS &NativeRegisterContextSunOS::GetProcess() {
+  return static_cast<NativeProcessSunOS &>(m_thread.GetProcess());
+}
+
+::pid_t NativeRegisterContextSunOS::GetProcessPid() {
+  return GetProcess().GetID();
+}
+
+struct ps_prochandle *NativeRegisterContextSunOS::GetProcHdl() {
+  return GetProcess().GetProcHdl();
+}
