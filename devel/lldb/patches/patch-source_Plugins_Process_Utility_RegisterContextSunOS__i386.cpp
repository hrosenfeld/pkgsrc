$NetBSD$

--- source/Plugins/Process/Utility/RegisterContextSunOS_i386.cpp.orig	2020-06-30 15:10:11.204888837 +0000
+++ source/Plugins/Process/Utility/RegisterContextSunOS_i386.cpp
@@ -0,0 +1,81 @@
+//===-- RegisterContextSunOS_i386.cpp ------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===---------------------------------------------------------------------===//
+
+#include "RegisterContextSunOS_i386.h"
+#include "RegisterContextPOSIX_x86.h"
+
+using namespace lldb_private;
+using namespace lldb;
+
+/* <sys/regset.h> */
+struct GPR {
+  uint32_t gs;     /*  0 */
+  uint32_t fs;     /*  1 */
+  uint32_t es;     /*  2 */
+  uint32_t ds;     /*  3 */
+  uint32_t edi;    /*  4 */
+  uint32_t esi;    /*  5 */
+  uint32_t ebp;    /*  6 */
+  uint32_t isp;    /*  7 */
+  uint32_t ebx;    /*  8 */
+  uint32_t edx;    /*  9 */
+  uint32_t ecx;    /* 10 */
+  uint32_t eax;    /* 11 */
+  uint32_t trapno; /* 12 */
+  uint32_t err;    /* 13 */
+  uint32_t eip;    /* 14 */
+  uint32_t cs;     /* 15 */
+  uint32_t eflags; /* 16 */
+  uint32_t esp;    /* 17 */
+  uint32_t ss;     /* 18 */
+};
+
+struct dbreg {
+  uint32_t dr[8]; /* debug registers */
+                  /* Index 0-3: debug address registers */
+                  /* Index 4-5: reserved */
+                  /* Index 6: debug status */
+                  /* Index 7: debug control */
+};
+
+using FPR_i386 = FXSAVE;
+
+struct UserArea {
+  GPR gpr;
+  FPR_i386 i387;
+  // No debug register access on Solaris/illumos.
+};
+
+#define DR_SIZE sizeof(uint32_t)
+#define DR_OFFSET(reg_index) (LLVM_EXTENSION offsetof(dbreg, dr[reg_index]))
+
+// Include RegisterInfos_i386 to declare our g_register_infos_i386 structure.
+#define DECLARE_REGISTER_INFOS_I386_STRUCT
+#include "RegisterInfos_i386.h"
+#undef DECLARE_REGISTER_INFOS_I386_STRUCT
+
+RegisterContextSunOS_i386::RegisterContextSunOS_i386(
+    const ArchSpec &target_arch)
+    : RegisterInfoInterface(target_arch) {}
+
+size_t RegisterContextSunOS_i386::GetGPRSize() const { return sizeof(GPR); }
+
+const RegisterInfo *RegisterContextSunOS_i386::GetRegisterInfo() const {
+  switch (m_target_arch.GetMachine()) {
+  case llvm::Triple::x86:
+    return g_register_infos_i386;
+  default:
+    assert(false && "Unhandled target architecture.");
+    return nullptr;
+  }
+}
+
+uint32_t RegisterContextSunOS_i386::GetRegisterCount() const {
+  return static_cast<uint32_t>(sizeof(g_register_infos_i386) /
+                               sizeof(g_register_infos_i386[0]));
+}
