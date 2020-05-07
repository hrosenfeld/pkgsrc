$NetBSD$

--- source/Plugins/Platform/SunOS/PlatformSunOS.h.orig	2020-04-01 12:12:33.180279502 +0000
+++ source/Plugins/Platform/SunOS/PlatformSunOS.h
@@ -0,0 +1,68 @@
+//===-- PlatformSunOS.h -----------------------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef liblldb_PlatformSunOS_h_
+#define liblldb_PlatformSunOS_h_
+
+#include "Plugins/Platform/POSIX/PlatformPOSIX.h"
+
+namespace lldb_private {
+namespace platform_sunos {
+
+class PlatformSunOS : public PlatformPOSIX {
+public:
+  PlatformSunOS(bool is_host);
+
+  ~PlatformSunOS() override;
+
+  static void Initialize();
+
+  static void Terminate();
+
+  // lldb_private::PluginInterface functions
+  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);
+
+  static ConstString GetPluginNameStatic(bool is_host);
+
+  static const char *GetPluginDescriptionStatic(bool is_host);
+
+  ConstString GetPluginName() override;
+
+  uint32_t GetPluginVersion() override { return 1; }
+
+  // lldb_private::Platform functions
+  const char *GetDescription() override {
+    return GetPluginDescriptionStatic(IsHost());
+  }
+
+  void GetStatus(Stream &strm) override;
+
+  bool GetSupportedArchitectureAtIndex(uint32_t idx, ArchSpec &arch) override;
+
+  int32_t GetResumeCountForLaunchInfo(ProcessLaunchInfo &launch_info) override;
+
+  bool CanDebugProcess() override;
+
+  lldb::ProcessSP DebugProcess(ProcessLaunchInfo &launch_info,
+                               Debugger &debugger, Target *target,
+                               Status &error) override;
+
+  void CalculateTrapHandlerSymbolNames() override;
+
+  MmapArgList GetMmapArgumentList(const ArchSpec &arch, lldb::addr_t addr,
+                                  lldb::addr_t length, unsigned prot,
+                                  unsigned flags, lldb::addr_t fd,
+                                  lldb::addr_t offset) override;
+private:
+  DISALLOW_COPY_AND_ASSIGN(PlatformSunOS);
+};
+
+} // namespace platform_sunos
+} // namespace lldb_private
+
+#endif // liblldb_PlatformSunOS_h_
