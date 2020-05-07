$NetBSD$

--- source/Plugins/Process/SunOS/NativeRegisterContextSunOS_x86_64.h.orig	2020-07-01 15:36:04.926477028 +0000
+++ source/Plugins/Process/SunOS/NativeRegisterContextSunOS_x86_64.h
@@ -0,0 +1,80 @@
+//===-- NativeRegisterContextSunOS_x86_64.h --------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#if defined(__x86_64__)
+
+#ifndef lldb_NativeRegisterContextSunOS_x86_64_h
+#define lldb_NativeRegisterContextSunOS_x86_64_h
+
+// clang-format off
+#include <sys/param.h>
+#include <sys/types.h>
+#include <procfs.h>
+// clang-format on
+
+#include "Plugins/Process/SunOS/NativeRegisterContextSunOS.h"
+#include "Plugins/Process/Utility/RegisterContext_x86.h"
+#include "Plugins/Process/Utility/lldb-x86-register-enums.h"
+
+namespace lldb_private {
+namespace process_sunos {
+
+class NativeProcessSunOS;
+
+class NativeRegisterContextSunOS_x86_64 : public NativeRegisterContextSunOS {
+public:
+  NativeRegisterContextSunOS_x86_64(const ArchSpec &target_arch,
+                                     NativeThreadProtocol &native_thread);
+  uint32_t GetRegisterSetCount() const override;
+
+  const RegisterSet *GetRegisterSet(uint32_t set_index) const override;
+
+  Status ReadRegister(const RegisterInfo *reg_info,
+                      RegisterValue &reg_value) override;
+
+  Status WriteRegister(const RegisterInfo *reg_info,
+                       const RegisterValue &reg_value) override;
+
+  Status ReadAllRegisterValues(lldb::DataBufferSP &data_sp) override;
+
+  Status WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;
+
+  uint32_t NumSupportedHardwareWatchpoints() override;
+
+private:
+  // Private member types.
+  enum { GPRegSet, FPRegSet, DBRegSet, XStateRegSet };
+
+  // Private member variables.
+  prgregset_t m_gpr_x86_64;
+  prfpregset_t m_fpr_x86_64;
+#ifdef HAVE_XSTATE
+  struct xstate m_xstate_x86_64;
+#endif
+
+  int GetSetForNativeRegNum(int reg_num) const;
+
+  Status ReadRegisterSet(uint32_t set);
+  Status WriteRegisterSet(uint32_t set);
+
+  Status ReadRegister_x86_64(const RegisterInfo *reg_info,
+                             RegisterValue &reg_value);
+  Status WriteRegister_x86_64(const RegisterInfo *reg_info,
+                              const RegisterValue &reg_value);
+  Status ReadRegister_i386(const RegisterInfo *reg_info,
+                           RegisterValue &reg_value);
+  Status WriteRegister_i386(const RegisterInfo *reg_info,
+                            const RegisterValue &reg_value);
+};
+
+} // namespace process_sunos
+} // namespace lldb_private
+
+#endif // #ifndef lldb_NativeRegisterContextSunOS_x86_64_h
+
+#endif // defined(__x86_64__)
