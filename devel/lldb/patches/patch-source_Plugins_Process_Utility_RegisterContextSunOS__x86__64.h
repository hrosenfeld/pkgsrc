$NetBSD$

--- source/Plugins/Process/Utility/RegisterContextSunOS_x86_64.h.orig	2020-04-08 16:27:03.687613896 +0000
+++ source/Plugins/Process/Utility/RegisterContextSunOS_x86_64.h
@@ -0,0 +1,30 @@
+//===-- RegisterContextSunOS_x86_64.h -------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef liblldb_RegisterContextSunOS_x86_64_H_
+#define liblldb_RegisterContextSunOS_x86_64_H_
+
+#include "RegisterInfoInterface.h"
+
+class RegisterContextSunOS_x86_64
+    : public lldb_private::RegisterInfoInterface {
+public:
+  RegisterContextSunOS_x86_64(const lldb_private::ArchSpec &target_arch);
+
+  size_t GetGPRSize() const override;
+
+  const lldb_private::RegisterInfo *GetRegisterInfo() const override;
+
+  uint32_t GetRegisterCount() const override;
+
+private:
+  const lldb_private::RegisterInfo *m_register_info_p;
+  const uint32_t m_register_count;
+};
+
+#endif
