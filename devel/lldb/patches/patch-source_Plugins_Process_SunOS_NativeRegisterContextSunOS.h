$NetBSD$

--- source/Plugins/Process/SunOS/NativeRegisterContextSunOS.h.orig	2020-07-02 14:13:00.135000851 +0000
+++ source/Plugins/Process/SunOS/NativeRegisterContextSunOS.h
@@ -0,0 +1,49 @@
+//===-- NativeRegisterContextSunOS.h ---------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef lldb_NativeRegisterContextSunOS_h
+#define lldb_NativeRegisterContextSunOS_h
+
+#include "lldb/Host/common/NativeThreadProtocol.h"
+
+#include "Plugins/Process/SunOS/NativeProcessSunOS.h"
+#include "Plugins/Process/Utility/NativeRegisterContextRegisterInfo.h"
+
+#include <libproc.h>
+
+namespace lldb_private {
+namespace process_sunos {
+
+class NativeRegisterContextSunOS : public NativeRegisterContextRegisterInfo {
+public:
+  NativeRegisterContextSunOS(NativeThreadProtocol &native_thread,
+                              RegisterInfoInterface *reg_info_interface_p);
+
+  // This function is implemented in the NativeRegisterContextSunOS_*
+  // subclasses to create a new instance of the host specific
+  // NativeRegisterContextSunOS. The implementations can't collide as only one
+  // NativeRegisterContextSunOS_* variant should be compiled into the final
+  // executable.
+  static NativeRegisterContextSunOS *
+  CreateHostNativeRegisterContextSunOS(const ArchSpec &target_arch,
+                                        NativeThreadProtocol &native_thread);
+
+protected:
+  Status GetGRegSet(prgregset_t buf);
+  Status GetFPRegSet(prfpregset_t *buf);
+  Status SetGRegSet(prgregset_t buf);
+  Status SetFPRegSet(prfpregset_t *buf);
+  virtual NativeProcessSunOS &GetProcess();
+  virtual ::pid_t GetProcessPid();
+  struct ps_prochandle *GetProcHdl();
+};
+
+} // namespace process_sunos
+} // namespace lldb_private
+
+#endif // #ifndef lldb_NativeRegisterContextSunOS_h
