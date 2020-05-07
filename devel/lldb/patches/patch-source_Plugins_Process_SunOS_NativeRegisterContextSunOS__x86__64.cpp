$NetBSD$

--- source/Plugins/Process/SunOS/NativeRegisterContextSunOS_x86_64.cpp.orig	2020-07-02 15:01:00.660242092 +0000
+++ source/Plugins/Process/SunOS/NativeRegisterContextSunOS_x86_64.cpp
@@ -0,0 +1,1188 @@
+//===-- NativeRegisterContextSunOS_x86_64.cpp ---------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#if defined(__x86_64__)
+
+#include "NativeRegisterContextSunOS_x86_64.h"
+
+#include "lldb/Host/HostInfo.h"
+#include "lldb/Utility/DataBufferHeap.h"
+#include "lldb/Utility/Log.h"
+#include "lldb/Utility/RegisterValue.h"
+#include "lldb/Utility/Status.h"
+
+#include "Plugins/Process/Utility/RegisterContextSunOS_i386.h"
+#include "Plugins/Process/Utility/RegisterContextSunOS_x86_64.h"
+
+// clang-format off
+#include <sys/types.h>
+#include <sys/uio.h>
+#include <elf.h>
+#include <err.h>
+#include <stdint.h>
+#include <stdlib.h>
+#include <libproc.h>
+// clang-format on
+
+using namespace lldb_private;
+using namespace lldb_private::process_sunos;
+
+// Private namespace.
+
+namespace {
+// x86 32-bit general purpose registers.
+const uint32_t g_gpr_regnums_i386[] = {
+    lldb_eax_i386,      lldb_ebx_i386,    lldb_ecx_i386, lldb_edx_i386,
+    lldb_edi_i386,      lldb_esi_i386,    lldb_ebp_i386, lldb_esp_i386,
+    lldb_eip_i386,      lldb_eflags_i386, lldb_cs_i386,  lldb_fs_i386,
+    lldb_gs_i386,       lldb_ss_i386,     lldb_ds_i386,  lldb_es_i386,
+    lldb_ax_i386,       lldb_bx_i386,     lldb_cx_i386,  lldb_dx_i386,
+    lldb_di_i386,       lldb_si_i386,     lldb_bp_i386,  lldb_sp_i386,
+    lldb_ah_i386,       lldb_bh_i386,     lldb_ch_i386,  lldb_dh_i386,
+    lldb_al_i386,       lldb_bl_i386,     lldb_cl_i386,  lldb_dl_i386,
+    LLDB_INVALID_REGNUM // register sets need to end with this flag
+};
+static_assert((sizeof(g_gpr_regnums_i386) / sizeof(g_gpr_regnums_i386[0])) -
+                      1 ==
+                  k_num_gpr_registers_i386,
+              "g_gpr_regnums_i386 has wrong number of register infos");
+
+// x86 32-bit floating point registers.
+const uint32_t g_fpu_regnums_i386[] = {
+    lldb_fctrl_i386,    lldb_fstat_i386,     lldb_ftag_i386,  lldb_fop_i386,
+    lldb_fiseg_i386,    lldb_fioff_i386,     lldb_foseg_i386, lldb_fooff_i386,
+    lldb_mxcsr_i386,    lldb_mxcsrmask_i386, lldb_st0_i386,   lldb_st1_i386,
+    lldb_st2_i386,      lldb_st3_i386,       lldb_st4_i386,   lldb_st5_i386,
+    lldb_st6_i386,      lldb_st7_i386,       lldb_mm0_i386,   lldb_mm1_i386,
+    lldb_mm2_i386,      lldb_mm3_i386,       lldb_mm4_i386,   lldb_mm5_i386,
+    lldb_mm6_i386,      lldb_mm7_i386,       lldb_xmm0_i386,  lldb_xmm1_i386,
+    lldb_xmm2_i386,     lldb_xmm3_i386,      lldb_xmm4_i386,  lldb_xmm5_i386,
+    lldb_xmm6_i386,     lldb_xmm7_i386,
+    LLDB_INVALID_REGNUM // register sets need to end with this flag
+};
+static_assert((sizeof(g_fpu_regnums_i386) / sizeof(g_fpu_regnums_i386[0])) -
+                      1 ==
+                  k_num_fpr_registers_i386,
+              "g_fpu_regnums_i386 has wrong number of register infos");
+
+// x86 64-bit general purpose registers.
+static const uint32_t g_gpr_regnums_x86_64[] = {
+    lldb_rax_x86_64,    lldb_rbx_x86_64,    lldb_rcx_x86_64, lldb_rdx_x86_64,
+    lldb_rdi_x86_64,    lldb_rsi_x86_64,    lldb_rbp_x86_64, lldb_rsp_x86_64,
+    lldb_r8_x86_64,     lldb_r9_x86_64,     lldb_r10_x86_64, lldb_r11_x86_64,
+    lldb_r12_x86_64,    lldb_r13_x86_64,    lldb_r14_x86_64, lldb_r15_x86_64,
+    lldb_rip_x86_64,    lldb_rflags_x86_64, lldb_cs_x86_64,  lldb_fs_x86_64,
+    lldb_gs_x86_64,     lldb_ss_x86_64,     lldb_ds_x86_64,  lldb_es_x86_64,
+    lldb_eax_x86_64,    lldb_ebx_x86_64,    lldb_ecx_x86_64, lldb_edx_x86_64,
+    lldb_edi_x86_64,    lldb_esi_x86_64,    lldb_ebp_x86_64, lldb_esp_x86_64,
+    lldb_r8d_x86_64,  // Low 32 bits or r8
+    lldb_r9d_x86_64,  // Low 32 bits or r9
+    lldb_r10d_x86_64, // Low 32 bits or r10
+    lldb_r11d_x86_64, // Low 32 bits or r11
+    lldb_r12d_x86_64, // Low 32 bits or r12
+    lldb_r13d_x86_64, // Low 32 bits or r13
+    lldb_r14d_x86_64, // Low 32 bits or r14
+    lldb_r15d_x86_64, // Low 32 bits or r15
+    lldb_ax_x86_64,     lldb_bx_x86_64,     lldb_cx_x86_64,  lldb_dx_x86_64,
+    lldb_di_x86_64,     lldb_si_x86_64,     lldb_bp_x86_64,  lldb_sp_x86_64,
+    lldb_r8w_x86_64,  // Low 16 bits or r8
+    lldb_r9w_x86_64,  // Low 16 bits or r9
+    lldb_r10w_x86_64, // Low 16 bits or r10
+    lldb_r11w_x86_64, // Low 16 bits or r11
+    lldb_r12w_x86_64, // Low 16 bits or r12
+    lldb_r13w_x86_64, // Low 16 bits or r13
+    lldb_r14w_x86_64, // Low 16 bits or r14
+    lldb_r15w_x86_64, // Low 16 bits or r15
+    lldb_ah_x86_64,     lldb_bh_x86_64,     lldb_ch_x86_64,  lldb_dh_x86_64,
+    lldb_al_x86_64,     lldb_bl_x86_64,     lldb_cl_x86_64,  lldb_dl_x86_64,
+    lldb_dil_x86_64,    lldb_sil_x86_64,    lldb_bpl_x86_64, lldb_spl_x86_64,
+    lldb_r8l_x86_64,    // Low 8 bits or r8
+    lldb_r9l_x86_64,    // Low 8 bits or r9
+    lldb_r10l_x86_64,   // Low 8 bits or r10
+    lldb_r11l_x86_64,   // Low 8 bits or r11
+    lldb_r12l_x86_64,   // Low 8 bits or r12
+    lldb_r13l_x86_64,   // Low 8 bits or r13
+    lldb_r14l_x86_64,   // Low 8 bits or r14
+    lldb_r15l_x86_64,   // Low 8 bits or r15
+    LLDB_INVALID_REGNUM // register sets need to end with this flag
+};
+static_assert((sizeof(g_gpr_regnums_x86_64) / sizeof(g_gpr_regnums_x86_64[0])) -
+                      1 ==
+                  k_num_gpr_registers_x86_64,
+              "g_gpr_regnums_x86_64 has wrong number of register infos");
+
+// x86 64-bit floating point registers.
+static const uint32_t g_fpu_regnums_x86_64[] = {
+    lldb_fctrl_x86_64,     lldb_fstat_x86_64, lldb_ftag_x86_64,
+    lldb_fop_x86_64,       lldb_fiseg_x86_64, lldb_fioff_x86_64,
+    lldb_foseg_x86_64,     lldb_fooff_x86_64, lldb_mxcsr_x86_64,
+    lldb_mxcsrmask_x86_64, lldb_st0_x86_64,   lldb_st1_x86_64,
+    lldb_st2_x86_64,       lldb_st3_x86_64,   lldb_st4_x86_64,
+    lldb_st5_x86_64,       lldb_st6_x86_64,   lldb_st7_x86_64,
+    lldb_mm0_x86_64,       lldb_mm1_x86_64,   lldb_mm2_x86_64,
+    lldb_mm3_x86_64,       lldb_mm4_x86_64,   lldb_mm5_x86_64,
+    lldb_mm6_x86_64,       lldb_mm7_x86_64,   lldb_xmm0_x86_64,
+    lldb_xmm1_x86_64,      lldb_xmm2_x86_64,  lldb_xmm3_x86_64,
+    lldb_xmm4_x86_64,      lldb_xmm5_x86_64,  lldb_xmm6_x86_64,
+    lldb_xmm7_x86_64,      lldb_xmm8_x86_64,  lldb_xmm9_x86_64,
+    lldb_xmm10_x86_64,     lldb_xmm11_x86_64, lldb_xmm12_x86_64,
+    lldb_xmm13_x86_64,     lldb_xmm14_x86_64, lldb_xmm15_x86_64,
+    LLDB_INVALID_REGNUM // register sets need to end with this flag
+};
+static_assert((sizeof(g_fpu_regnums_x86_64) / sizeof(g_fpu_regnums_x86_64[0])) -
+                      1 ==
+                  k_num_fpr_registers_x86_64,
+              "g_fpu_regnums_x86_64 has wrong number of register infos");
+
+// Number of register sets provided by this context.
+enum { k_num_extended_register_sets = 0, k_num_register_sets = 2 };
+
+// Register sets for x86 32-bit.
+static const RegisterSet g_reg_sets_i386[k_num_register_sets] = {
+    {"General Purpose Registers", "gpr", k_num_gpr_registers_i386,
+     g_gpr_regnums_i386},
+    {"Floating Point Registers", "fpu", k_num_fpr_registers_i386,
+     g_fpu_regnums_i386},
+};
+
+// Register sets for x86 64-bit.
+static const RegisterSet g_reg_sets_x86_64[k_num_register_sets] = {
+    {"General Purpose Registers", "gpr", k_num_gpr_registers_x86_64,
+     g_gpr_regnums_x86_64},
+    {"Floating Point Registers", "fpu", k_num_fpr_registers_x86_64,
+     g_fpu_regnums_x86_64},
+};
+
+#define REG_CONTEXT_SIZE (GetRegisterInfoInterface().GetGPRSize())
+} // namespace
+
+NativeRegisterContextSunOS *
+NativeRegisterContextSunOS::CreateHostNativeRegisterContextSunOS(
+    const ArchSpec &target_arch, NativeThreadProtocol &native_thread) {
+  return new NativeRegisterContextSunOS_x86_64(target_arch, native_thread);
+}
+
+// NativeRegisterContextSunOS_x86_64 members.
+
+static RegisterInfoInterface *
+CreateRegisterInfoInterface(const ArchSpec &target_arch) {
+  if (HostInfo::GetArchitecture().GetAddressByteSize() == 4) {
+    // 32-bit hosts run with a RegisterContextSunOS_i386 context.
+    return new RegisterContextSunOS_i386(target_arch);
+  } else {
+    assert((HostInfo::GetArchitecture().GetAddressByteSize() == 8) &&
+           "Register setting path assumes this is a 64-bit host");
+    // X86_64 hosts know how to work with 64-bit and 32-bit EXEs using the
+    // x86_64 register context.
+    return new RegisterContextSunOS_x86_64(target_arch);
+  }
+}
+
+NativeRegisterContextSunOS_x86_64::NativeRegisterContextSunOS_x86_64(
+    const ArchSpec &target_arch, NativeThreadProtocol &native_thread)
+    : NativeRegisterContextSunOS(native_thread,
+                                  CreateRegisterInfoInterface(target_arch)),
+      m_gpr_x86_64(), m_fpr_x86_64() {
+}
+
+// CONSIDER after local and llgs debugging are merged, register set support can
+// be moved into a base x86-64 class with IsRegisterSetAvailable made virtual.
+uint32_t NativeRegisterContextSunOS_x86_64::GetRegisterSetCount() const {
+  uint32_t sets = 0;
+  for (uint32_t set_index = 0; set_index < k_num_register_sets; ++set_index) {
+    if (GetSetForNativeRegNum(set_index) != -1)
+      ++sets;
+  }
+
+  return sets;
+}
+
+const RegisterSet *
+NativeRegisterContextSunOS_x86_64::GetRegisterSet(uint32_t set_index) const {
+  switch (GetRegisterInfoInterface().GetTargetArchitecture().GetMachine()) {
+  case llvm::Triple::x86:
+    return &g_reg_sets_i386[set_index];
+  case llvm::Triple::x86_64:
+    return &g_reg_sets_x86_64[set_index];
+  default:
+    assert(false && "Unhandled target architecture.");
+    return nullptr;
+  }
+}
+
+int NativeRegisterContextSunOS_x86_64::GetSetForNativeRegNum(
+    int reg_num) const {
+
+  switch (GetRegisterInfoInterface().GetTargetArchitecture().GetMachine()) {
+  case llvm::Triple::x86:
+    if (reg_num <= k_last_gpr_i386)
+      return GPRegSet;
+    else if (reg_num <= k_last_fpr_i386)
+      return FPRegSet;
+    else if (reg_num <= k_last_avx_i386)
+      return XStateRegSet; // AVX
+    else if (reg_num <= k_last_mpxr_i386)
+      return -1; // MPXR
+    else if (reg_num <= k_last_mpxc_i386)
+      return -1; // MPXC
+    else if (reg_num <= lldb_dr7_i386)
+      return DBRegSet; // DBR
+    else
+      return -1;
+  case llvm::Triple::x86_64:
+    if (reg_num <= k_last_gpr_x86_64)
+      return GPRegSet;
+    else if (reg_num <= k_last_fpr_x86_64)
+      return FPRegSet;
+    else if (reg_num <= k_last_avx_x86_64)
+      return XStateRegSet; // AVX
+    else if (reg_num <= k_last_mpxr_x86_64)
+      return -1; // MPXR
+    else if (reg_num <= k_last_mpxc_x86_64)
+      return -1; // MPXC
+    else if (reg_num <= lldb_dr7_x86_64)
+      return DBRegSet; // DBR
+    else
+      return -1;
+  default:
+    assert(false && "Unhandled target architecture.");
+    return -1;
+  }
+
+}
+
+Status NativeRegisterContextSunOS_x86_64::ReadRegisterSet(uint32_t set) {
+  switch (set) {
+  case GPRegSet:
+    return GetGRegSet(m_gpr_x86_64);
+  case FPRegSet:
+    return GetFPRegSet(&m_fpr_x86_64);
+  case DBRegSet:
+    return Status("Debug registers not supported by the kernel");
+  case XStateRegSet:
+#ifdef HAVE_XSTATE
+    {
+      struct iovec iov = {&m_xstate_x86_64, sizeof(m_xstate_x86_64)};
+      return DoRegisterSet(PT_GETXSTATE, &iov);
+    }
+#else
+    return Status("XState is not supported by the kernel");
+#endif
+  }
+  llvm_unreachable("NativeRegisterContextSunOS_x86_64::ReadRegisterSet");
+}
+
+Status NativeRegisterContextSunOS_x86_64::WriteRegisterSet(uint32_t set) {
+  switch (set) {
+  case GPRegSet:
+    return SetGRegSet(m_gpr_x86_64);
+  case FPRegSet:
+    return SetFPRegSet(&m_fpr_x86_64);
+  case DBRegSet:
+    return Status("Debug registers not supported by the kernel");
+  case XStateRegSet:
+#ifdef HAVE_XSTATE
+    {
+      struct iovec iov = {&m_xstate_x86_64, sizeof(m_xstate_x86_64)};
+      return DoRegisterSet(PT_SETXSTATE, &iov);
+    }
+#else
+    return Status("XState is not supported by the kernel");
+#endif
+  }
+  llvm_unreachable("NativeRegisterContextSunOS_x86_64::WriteRegisterSet");
+}
+
+Status
+NativeRegisterContextSunOS_x86_64::ReadRegister(const RegisterInfo *reg_info,
+                                                 RegisterValue &reg_value) {
+  Status error;
+
+  if (!reg_info) {
+    error.SetErrorString("reg_info NULL");
+    return error;
+  }
+
+  const uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
+  if (reg == LLDB_INVALID_REGNUM) {
+    // This is likely an internal register for lldb use only and should not be
+    // directly queried.
+    error.SetErrorStringWithFormat("register \"%s\" is an internal-only lldb "
+                                   "register, cannot read directly",
+                                   reg_info->name);
+    return error;
+  }
+
+  int set = GetSetForNativeRegNum(reg);
+  if (set == -1) {
+    // This is likely an internal register for lldb use only and should not be
+    // directly queried.
+    error.SetErrorStringWithFormat("register \"%s\" is in unrecognized set",
+                                   reg_info->name);
+    return error;
+  }
+
+  error = ReadRegisterSet(set);
+  if (error.Fail())
+    return error;
+
+  switch (GetRegisterInfoInterface().GetTargetArchitecture().GetMachine()) {
+  case llvm::Triple::x86:
+    return ReadRegister_i386(reg_info, reg_value);
+  case llvm::Triple::x86_64:
+    return ReadRegister_x86_64(reg_info, reg_value);
+  default:
+    return Status("Unhandled target architecture.");
+  }
+}
+
+
+Status NativeRegisterContextSunOS_x86_64::ReadRegister_i386(
+    const RegisterInfo *reg_info , RegisterValue &reg_value) {
+  Status error;
+
+  const uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
+
+  switch (reg) {
+  case lldb_eax_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RAX];
+    break;
+  case lldb_ebx_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RBX];
+    break;
+  case lldb_ecx_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RCX];
+    break;
+  case lldb_edx_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RDX];
+    break;
+  case lldb_edi_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RDI];
+    break;
+  case lldb_esi_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RSI];
+    break;
+  case lldb_ebp_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RBP];
+    break;
+  case lldb_esp_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RSP];
+    break;
+  case lldb_eip_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RIP];
+    break;
+  case lldb_eflags_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_RFL];
+    break;
+  case lldb_cs_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_CS];
+    break;
+  case lldb_fs_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_FS];
+    break;
+  case lldb_gs_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_GS];
+    break;
+  case lldb_ss_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_SS];
+    break;
+  case lldb_ds_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_DS];
+    break;
+  case lldb_es_i386:
+    reg_value = (uint32_t)m_gpr_x86_64[REG_ES];
+    break;
+  case lldb_fctrl_i386:
+    reg_value = (uint16_t)m_fpr_x86_64.fp_reg_set.fpchip_state.cw;
+    break;
+  case lldb_fstat_i386:
+    reg_value = (uint16_t)m_fpr_x86_64.fp_reg_set.fpchip_state.sw;
+    break;
+  case lldb_ftag_i386:
+    reg_value = (uint16_t)m_fpr_x86_64.fp_reg_set.fpchip_state.fctw;
+    break;
+  case lldb_fop_i386:
+    reg_value = (uint64_t)m_fpr_x86_64.fp_reg_set.fpchip_state.fop;
+    break;
+  case lldb_fiseg_i386:
+    reg_value = (uint32_t)(m_fpr_x86_64.fp_reg_set.fpchip_state.rip >> 32);
+    break;
+  case lldb_fioff_i386:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.rip;
+    break;
+  case lldb_foseg_i386:
+    reg_value = (uint64_t)(m_fpr_x86_64.fp_reg_set.fpchip_state.rdp >> 32);
+    break;
+  case lldb_fooff_i386:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.rdp;
+    break;
+  case lldb_mxcsr_i386:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr;
+    break;
+  case lldb_mxcsrmask_i386:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr_mask;
+    break;
+  case lldb_st0_i386:
+  case lldb_st1_i386:
+  case lldb_st2_i386:
+  case lldb_st3_i386:
+  case lldb_st4_i386:
+  case lldb_st5_i386:
+  case lldb_st6_i386:
+  case lldb_st7_i386:
+    reg_value.SetBytes(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_st0_i386],
+                       reg_info->byte_size, endian::InlHostByteOrder());
+    break;
+  case lldb_mm0_i386:
+  case lldb_mm1_i386:
+  case lldb_mm2_i386:
+  case lldb_mm3_i386:
+  case lldb_mm4_i386:
+  case lldb_mm5_i386:
+  case lldb_mm6_i386:
+  case lldb_mm7_i386:
+    reg_value.SetBytes(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_mm0_i386],
+                       reg_info->byte_size, endian::InlHostByteOrder());
+    break;
+  case lldb_xmm0_i386:
+  case lldb_xmm1_i386:
+  case lldb_xmm2_i386:
+  case lldb_xmm3_i386:
+  case lldb_xmm4_i386:
+  case lldb_xmm5_i386:
+  case lldb_xmm6_i386:
+  case lldb_xmm7_i386:
+    reg_value.SetBytes(&m_fpr_x86_64.fp_reg_set.fpchip_state.xmm[reg - lldb_xmm0_i386],
+                       reg_info->byte_size, endian::InlHostByteOrder());
+    break;
+  case lldb_ymm0_i386:
+  case lldb_ymm1_i386:
+  case lldb_ymm2_i386:
+  case lldb_ymm3_i386:
+  case lldb_ymm4_i386:
+  case lldb_ymm5_i386:
+  case lldb_ymm6_i386:
+  case lldb_ymm7_i386:
+#ifdef HAVE_XSTATE
+    if (!(m_xstate_x86_64.xs_rfbm & XCR0_SSE) ||
+        !(m_xstate_x86_64.xs_rfbm & XCR0_YMM_Hi128)) {
+      error.SetErrorStringWithFormat("register \"%s\" not supported by CPU/kernel",
+                                     reg_info->name);
+    } else {
+      uint32_t reg_index = reg - lldb_ymm0_i386;
+      YMMReg ymm = XStateToYMM(
+          m_xstate_x86_64.xs_fxsave.fx_xmm[reg_index].xmm_bytes,
+          m_xstate_x86_64.xs_ymm_hi128.xs_ymm[reg_index].ymm_bytes);
+      reg_value.SetBytes(ymm.bytes, reg_info->byte_size,
+                         endian::InlHostByteOrder());
+    }
+#else
+    error.SetErrorString("XState queries not supported by the kernel");
+#endif
+    break;
+  case lldb_dr0_i386:
+  case lldb_dr1_i386:
+  case lldb_dr2_i386:
+  case lldb_dr3_i386:
+  case lldb_dr4_i386:
+  case lldb_dr5_i386:
+  case lldb_dr6_i386:
+  case lldb_dr7_i386:
+    error.SetErrorString("debug register queries not supported by the kernel");
+    break;
+  default:
+    error.SetErrorStringWithFormat("unknown register \"%s\"", reg_info->name);
+  }
+
+  return error;
+}
+
+Status NativeRegisterContextSunOS_x86_64::ReadRegister_x86_64(
+    const RegisterInfo *reg_info , RegisterValue &reg_value) {
+  Status error;
+
+  const uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
+
+  switch (reg) {
+  case lldb_rax_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RAX];
+    break;
+  case lldb_rbx_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RBX];
+    break;
+  case lldb_rcx_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RCX];
+    break;
+  case lldb_rdx_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RDX];
+    break;
+  case lldb_rdi_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RDI];
+    break;
+  case lldb_rsi_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RSI];
+    break;
+  case lldb_rbp_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RBP];
+    break;
+  case lldb_rsp_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RSP];
+    break;
+  case lldb_r8_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R8];
+    break;
+  case lldb_r9_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R9];
+    break;
+  case lldb_r10_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R10];
+    break;
+  case lldb_r11_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R11];
+    break;
+  case lldb_r12_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R12];
+    break;
+  case lldb_r13_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R13];
+    break;
+  case lldb_r14_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R14];
+    break;
+  case lldb_r15_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_R15];
+    break;
+  case lldb_rip_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RIP];
+    break;
+  case lldb_rflags_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_RFL];
+    break;
+  case lldb_cs_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_CS];
+    break;
+  case lldb_fs_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_FS];
+    break;
+  case lldb_gs_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_GS];
+    break;
+  case lldb_ss_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_SS];
+    break;
+  case lldb_ds_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_DS];
+    break;
+  case lldb_es_x86_64:
+    reg_value = (uint64_t)m_gpr_x86_64[REG_ES];
+    break;
+  case lldb_fctrl_x86_64:
+    reg_value = (uint16_t)m_fpr_x86_64.fp_reg_set.fpchip_state.cw;
+    break;
+  case lldb_fstat_x86_64:
+    reg_value = (uint16_t)m_fpr_x86_64.fp_reg_set.fpchip_state.sw;
+    break;
+  case lldb_ftag_x86_64:
+    reg_value = (uint16_t)m_fpr_x86_64.fp_reg_set.fpchip_state.fctw;
+    break;
+  case lldb_fop_x86_64:
+    reg_value = (uint64_t)m_fpr_x86_64.fp_reg_set.fpchip_state.fop;
+    break;
+  case lldb_fiseg_x86_64:
+    reg_value = (uint32_t)(m_fpr_x86_64.fp_reg_set.fpchip_state.rip >> 32);
+    break;
+  case lldb_fioff_x86_64:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.rip;
+    break;
+  case lldb_foseg_x86_64:
+    reg_value = (uint64_t)(m_fpr_x86_64.fp_reg_set.fpchip_state.rdp >> 32);
+    break;
+  case lldb_fooff_x86_64:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.rdp;
+    break;
+  case lldb_mxcsr_x86_64:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr;
+    break;
+  case lldb_mxcsrmask_x86_64:
+    reg_value = (uint32_t)m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr_mask;
+    break;
+  case lldb_st0_x86_64:
+  case lldb_st1_x86_64:
+  case lldb_st2_x86_64:
+  case lldb_st3_x86_64:
+  case lldb_st4_x86_64:
+  case lldb_st5_x86_64:
+  case lldb_st6_x86_64:
+  case lldb_st7_x86_64:
+    reg_value.SetBytes(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_st0_x86_64],
+                       reg_info->byte_size, endian::InlHostByteOrder());
+    break;
+  case lldb_mm0_x86_64:
+  case lldb_mm1_x86_64:
+  case lldb_mm2_x86_64:
+  case lldb_mm3_x86_64:
+  case lldb_mm4_x86_64:
+  case lldb_mm5_x86_64:
+  case lldb_mm6_x86_64:
+  case lldb_mm7_x86_64:
+    reg_value.SetBytes(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_mm0_x86_64],
+                       reg_info->byte_size, endian::InlHostByteOrder());
+    break;
+  case lldb_xmm0_x86_64:
+  case lldb_xmm1_x86_64:
+  case lldb_xmm2_x86_64:
+  case lldb_xmm3_x86_64:
+  case lldb_xmm4_x86_64:
+  case lldb_xmm5_x86_64:
+  case lldb_xmm6_x86_64:
+  case lldb_xmm7_x86_64:
+  case lldb_xmm8_x86_64:
+  case lldb_xmm9_x86_64:
+  case lldb_xmm10_x86_64:
+  case lldb_xmm11_x86_64:
+  case lldb_xmm12_x86_64:
+  case lldb_xmm13_x86_64:
+  case lldb_xmm14_x86_64:
+  case lldb_xmm15_x86_64:
+    reg_value.SetBytes(&m_fpr_x86_64.fp_reg_set.fpchip_state.xmm[reg - lldb_xmm0_x86_64],
+                       reg_info->byte_size, endian::InlHostByteOrder());
+    break;
+  case lldb_ymm0_x86_64:
+  case lldb_ymm1_x86_64:
+  case lldb_ymm2_x86_64:
+  case lldb_ymm3_x86_64:
+  case lldb_ymm4_x86_64:
+  case lldb_ymm5_x86_64:
+  case lldb_ymm6_x86_64:
+  case lldb_ymm7_x86_64:
+  case lldb_ymm8_x86_64:
+  case lldb_ymm9_x86_64:
+  case lldb_ymm10_x86_64:
+  case lldb_ymm11_x86_64:
+  case lldb_ymm12_x86_64:
+  case lldb_ymm13_x86_64:
+  case lldb_ymm14_x86_64:
+  case lldb_ymm15_x86_64:
+#ifdef HAVE_XSTATE
+    if (!(m_xstate_x86_64.xs_rfbm & XCR0_SSE) ||
+        !(m_xstate_x86_64.xs_rfbm & XCR0_YMM_Hi128)) {
+      error.SetErrorStringWithFormat("register \"%s\" not supported by CPU/kernel",
+                                     reg_info->name);
+    } else {
+      uint32_t reg_index = reg - lldb_ymm0_x86_64;
+      YMMReg ymm = XStateToYMM(
+          m_xstate_x86_64.xs_fxsave.fx_xmm[reg_index].xmm_bytes,
+          m_xstate_x86_64.xs_ymm_hi128.xs_ymm[reg_index].ymm_bytes);
+      reg_value.SetBytes(ymm.bytes, reg_info->byte_size,
+                         endian::InlHostByteOrder());
+    }
+#else
+    error.SetErrorString("XState queries not supported by the kernel");
+#endif
+    break;
+  case lldb_dr0_x86_64:
+  case lldb_dr1_x86_64:
+  case lldb_dr2_x86_64:
+  case lldb_dr3_x86_64:
+  case lldb_dr4_x86_64:
+  case lldb_dr5_x86_64:
+  case lldb_dr6_x86_64:
+  case lldb_dr7_x86_64:
+    error.SetErrorString("debug register queries not supported by the kernel");
+    break;
+  default:
+    error.SetErrorStringWithFormat("unknown register \"%s\"", reg_info->name);
+  }
+
+  return error;
+}
+
+Status NativeRegisterContextSunOS_x86_64::WriteRegister(
+    const RegisterInfo *reg_info, const RegisterValue &reg_value) {
+
+  Status error;
+
+  if (!reg_info) {
+    error.SetErrorString("reg_info NULL");
+    return error;
+  }
+
+  const uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
+  if (reg == LLDB_INVALID_REGNUM) {
+    // This is likely an internal register for lldb use only and should not be
+    // directly queried.
+    error.SetErrorStringWithFormat("register \"%s\" is an internal-only lldb "
+                                   "register, cannot read directly",
+                                   reg_info->name);
+    return error;
+  }
+
+  int set = GetSetForNativeRegNum(reg);
+  if (set == -1) {
+    // This is likely an internal register for lldb use only and should not be
+    // directly queried.
+    error.SetErrorStringWithFormat("register \"%s\" is in unrecognized set",
+                                   reg_info->name);
+    return error;
+  }
+
+  error = ReadRegisterSet(set);
+  if (error.Fail())
+    return error;
+
+  switch (GetRegisterInfoInterface().GetTargetArchitecture().GetMachine()) {
+  case llvm::Triple::x86:
+    error = WriteRegister_i386(reg_info, reg_value);
+    break;
+  case llvm::Triple::x86_64:
+    error = WriteRegister_x86_64(reg_info, reg_value);
+    break;
+  default:
+    error.SetErrorString("Unhandled target architecture.");
+  }
+
+  if (error.Fail())
+    return error;
+
+  return WriteRegisterSet(set);
+}
+
+Status NativeRegisterContextSunOS_x86_64::WriteRegister_i386(
+    const RegisterInfo *reg_info, const RegisterValue &reg_value) {
+  Status error;
+
+  uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
+
+  switch (reg) {
+  case lldb_eax_i386:
+    m_gpr_x86_64[REG_RAX] = reg_value.GetAsUInt32();
+    break;
+  case lldb_ebx_i386:
+    m_gpr_x86_64[REG_RBX] = reg_value.GetAsUInt32();
+    break;
+  case lldb_ecx_i386:
+    m_gpr_x86_64[REG_RCX] = reg_value.GetAsUInt32();
+    break;
+  case lldb_edx_i386:
+    m_gpr_x86_64[REG_RDX] = reg_value.GetAsUInt32();
+    break;
+  case lldb_edi_i386:
+    m_gpr_x86_64[REG_RDI] = reg_value.GetAsUInt32();
+    break;
+  case lldb_esi_i386:
+    m_gpr_x86_64[REG_RSI] = reg_value.GetAsUInt32();
+    break;
+  case lldb_ebp_i386:
+    m_gpr_x86_64[REG_RBP] = reg_value.GetAsUInt32();
+    break;
+  case lldb_esp_i386:
+    m_gpr_x86_64[REG_RSP] = reg_value.GetAsUInt32();
+    break;
+  case lldb_eip_i386:
+    m_gpr_x86_64[REG_RIP] = reg_value.GetAsUInt32();
+    break;
+  case lldb_eflags_i386:
+    m_gpr_x86_64[REG_RFL] = reg_value.GetAsUInt32();
+    break;
+  case lldb_cs_i386:
+    m_gpr_x86_64[REG_CS] = reg_value.GetAsUInt32();
+    break;
+  case lldb_fs_i386:
+    m_gpr_x86_64[REG_FS] = reg_value.GetAsUInt32();
+    break;
+  case lldb_gs_i386:
+    m_gpr_x86_64[REG_GS] = reg_value.GetAsUInt32();
+    break;
+  case lldb_ss_i386:
+    m_gpr_x86_64[REG_SS] = reg_value.GetAsUInt32();
+    break;
+  case lldb_ds_i386:
+    m_gpr_x86_64[REG_DS] = reg_value.GetAsUInt32();
+    break;
+  case lldb_es_i386:
+    m_gpr_x86_64[REG_ES] = reg_value.GetAsUInt32();
+    break;
+  case lldb_fctrl_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.cw = reg_value.GetAsUInt16();
+    break;
+  case lldb_fstat_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.sw = reg_value.GetAsUInt16();
+    break;
+  case lldb_ftag_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.fctw = (uint8_t)reg_value.GetAsUInt16();
+    break;
+  case lldb_fop_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.fop = reg_value.GetAsUInt16();
+    break;
+  case lldb_fiseg_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip &= 0xffffffffULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip |= reg_value.GetAsUInt64() << 32;
+    break;
+  case lldb_fioff_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip &= 0xffffffff00000000ULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip |= reg_value.GetAsUInt32();
+    break;
+  case lldb_foseg_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp &= 0xffffffffULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp |= reg_value.GetAsUInt64() << 32;
+    break;
+  case lldb_fooff_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp &= 0xffffffff00000000ULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp |= reg_value.GetAsUInt32();
+    break;
+  case lldb_mxcsr_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr = reg_value.GetAsUInt32();
+    break;
+  case lldb_mxcsrmask_i386:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr_mask = reg_value.GetAsUInt32();
+    break;
+  case lldb_st0_i386:
+  case lldb_st1_i386:
+  case lldb_st2_i386:
+  case lldb_st3_i386:
+  case lldb_st4_i386:
+  case lldb_st5_i386:
+  case lldb_st6_i386:
+  case lldb_st7_i386:
+    ::memcpy(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_st0_i386],
+             reg_value.GetBytes(), reg_value.GetByteSize());
+    break;
+  case lldb_mm0_i386:
+  case lldb_mm1_i386:
+  case lldb_mm2_i386:
+  case lldb_mm3_i386:
+  case lldb_mm4_i386:
+  case lldb_mm5_i386:
+  case lldb_mm6_i386:
+  case lldb_mm7_i386:
+    ::memcpy(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_mm0_i386],
+             reg_value.GetBytes(), reg_value.GetByteSize());
+    break;
+  case lldb_xmm0_i386:
+  case lldb_xmm1_i386:
+  case lldb_xmm2_i386:
+  case lldb_xmm3_i386:
+  case lldb_xmm4_i386:
+  case lldb_xmm5_i386:
+  case lldb_xmm6_i386:
+  case lldb_xmm7_i386:
+    ::memcpy(&m_fpr_x86_64.fp_reg_set.fpchip_state.xmm[reg - lldb_xmm0_i386],
+             reg_value.GetBytes(), reg_value.GetByteSize());
+    break;
+  case lldb_ymm0_i386:
+  case lldb_ymm1_i386:
+  case lldb_ymm2_i386:
+  case lldb_ymm3_i386:
+  case lldb_ymm4_i386:
+  case lldb_ymm5_i386:
+  case lldb_ymm6_i386:
+  case lldb_ymm7_i386:
+#ifdef HAVE_XSTATE
+    if (!(m_xstate_x86_64.xs_rfbm & XCR0_SSE) ||
+        !(m_xstate_x86_64.xs_rfbm & XCR0_YMM_Hi128)) {
+      error.SetErrorStringWithFormat("register \"%s\" not supported by CPU/kernel",
+                                     reg_info->name);
+    } else {
+      uint32_t reg_index = reg - lldb_ymm0_i386;
+      YMMReg ymm;
+      ::memcpy(ymm.bytes, reg_value.GetBytes(), reg_value.GetByteSize());
+      YMMToXState(ymm,
+          m_xstate_x86_64.xs_fxsave.fx_xmm[reg_index].xmm_bytes,
+          m_xstate_x86_64.xs_ymm_hi128.xs_ymm[reg_index].ymm_bytes);
+    }
+#else
+    error.SetErrorString("XState not supported by the kernel");
+#endif
+    break;
+  case lldb_dr0_i386:
+  case lldb_dr1_i386:
+  case lldb_dr2_i386:
+  case lldb_dr3_i386:
+  case lldb_dr4_i386:
+  case lldb_dr5_i386:
+  case lldb_dr6_i386:
+  case lldb_dr7_i386:
+    error.SetErrorString("debug registers not supported by the kernel");
+    break;
+  default:
+    error.SetErrorStringWithFormat("unknown register \"%s\"", reg_info->name);
+  }
+
+  return error;
+}
+
+Status NativeRegisterContextSunOS_x86_64::WriteRegister_x86_64(
+    const RegisterInfo *reg_info, const RegisterValue &reg_value) {
+  Status error;
+
+  uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
+
+  switch (reg) {
+  case lldb_rax_x86_64:
+    m_gpr_x86_64[REG_RAX] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rbx_x86_64:
+    m_gpr_x86_64[REG_RBX] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rcx_x86_64:
+    m_gpr_x86_64[REG_RCX] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rdx_x86_64:
+    m_gpr_x86_64[REG_RDX] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rdi_x86_64:
+    m_gpr_x86_64[REG_RDI] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rsi_x86_64:
+    m_gpr_x86_64[REG_RSI] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rbp_x86_64:
+    m_gpr_x86_64[REG_RBP] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rsp_x86_64:
+    m_gpr_x86_64[REG_RSP] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r8_x86_64:
+    m_gpr_x86_64[REG_R8] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r9_x86_64:
+    m_gpr_x86_64[REG_R9] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r10_x86_64:
+    m_gpr_x86_64[REG_R10] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r11_x86_64:
+    m_gpr_x86_64[REG_R11] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r12_x86_64:
+    m_gpr_x86_64[REG_R12] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r13_x86_64:
+    m_gpr_x86_64[REG_R13] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r14_x86_64:
+    m_gpr_x86_64[REG_R14] = reg_value.GetAsUInt64();
+    break;
+  case lldb_r15_x86_64:
+    m_gpr_x86_64[REG_R15] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rip_x86_64:
+    m_gpr_x86_64[REG_RIP] = reg_value.GetAsUInt64();
+    break;
+  case lldb_rflags_x86_64:
+    m_gpr_x86_64[REG_RFL] = reg_value.GetAsUInt64();
+    break;
+  case lldb_cs_x86_64:
+    m_gpr_x86_64[REG_CS] = reg_value.GetAsUInt64();
+    break;
+  case lldb_fs_x86_64:
+    m_gpr_x86_64[REG_FS] = reg_value.GetAsUInt64();
+    break;
+  case lldb_gs_x86_64:
+    m_gpr_x86_64[REG_GS] = reg_value.GetAsUInt64();
+    break;
+  case lldb_ss_x86_64:
+    m_gpr_x86_64[REG_SS] = reg_value.GetAsUInt64();
+    break;
+  case lldb_ds_x86_64:
+    m_gpr_x86_64[REG_DS] = reg_value.GetAsUInt64();
+    break;
+  case lldb_es_x86_64:
+    m_gpr_x86_64[REG_ES] = reg_value.GetAsUInt64();
+    break;
+  case lldb_fctrl_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.cw = reg_value.GetAsUInt16();
+    break;
+  case lldb_fstat_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.sw = reg_value.GetAsUInt16();
+    break;
+  case lldb_ftag_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.fctw = (uint8_t)reg_value.GetAsUInt16();
+    break;
+  case lldb_fop_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.fop = reg_value.GetAsUInt16();
+    break;
+  case lldb_fiseg_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip &= 0xffffffffULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip |= reg_value.GetAsUInt64() << 32;
+    break;
+  case lldb_fioff_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip &= 0xffffffff00000000ULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rip |= reg_value.GetAsUInt32();
+    break;
+  case lldb_foseg_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp &= 0xffffffffULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp |= reg_value.GetAsUInt64() << 32;
+    break;
+  case lldb_fooff_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp &= 0xffffffff00000000ULL;
+    m_fpr_x86_64.fp_reg_set.fpchip_state.rdp |= reg_value.GetAsUInt32();
+    break;
+  case lldb_mxcsr_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr = reg_value.GetAsUInt32();
+    break;
+  case lldb_mxcsrmask_x86_64:
+    m_fpr_x86_64.fp_reg_set.fpchip_state.mxcsr_mask = reg_value.GetAsUInt32();
+    break;
+  case lldb_st0_x86_64:
+  case lldb_st1_x86_64:
+  case lldb_st2_x86_64:
+  case lldb_st3_x86_64:
+  case lldb_st4_x86_64:
+  case lldb_st5_x86_64:
+  case lldb_st6_x86_64:
+  case lldb_st7_x86_64:
+    ::memcpy(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_st0_x86_64],
+             reg_value.GetBytes(), reg_value.GetByteSize());
+    break;
+  case lldb_mm0_x86_64:
+  case lldb_mm1_x86_64:
+  case lldb_mm2_x86_64:
+  case lldb_mm3_x86_64:
+  case lldb_mm4_x86_64:
+  case lldb_mm5_x86_64:
+  case lldb_mm6_x86_64:
+  case lldb_mm7_x86_64:
+    ::memcpy(&m_fpr_x86_64.fp_reg_set.fpchip_state.st[reg - lldb_mm0_x86_64],
+             reg_value.GetBytes(), reg_value.GetByteSize());
+    break;
+  case lldb_xmm0_x86_64:
+  case lldb_xmm1_x86_64:
+  case lldb_xmm2_x86_64:
+  case lldb_xmm3_x86_64:
+  case lldb_xmm4_x86_64:
+  case lldb_xmm5_x86_64:
+  case lldb_xmm6_x86_64:
+  case lldb_xmm7_x86_64:
+  case lldb_xmm8_x86_64:
+  case lldb_xmm9_x86_64:
+  case lldb_xmm10_x86_64:
+  case lldb_xmm11_x86_64:
+  case lldb_xmm12_x86_64:
+  case lldb_xmm13_x86_64:
+  case lldb_xmm14_x86_64:
+  case lldb_xmm15_x86_64:
+    ::memcpy(&m_fpr_x86_64.fp_reg_set.fpchip_state.xmm[reg - lldb_xmm0_x86_64],
+             reg_value.GetBytes(), reg_value.GetByteSize());
+    break;
+  case lldb_ymm0_x86_64:
+  case lldb_ymm1_x86_64:
+  case lldb_ymm2_x86_64:
+  case lldb_ymm3_x86_64:
+  case lldb_ymm4_x86_64:
+  case lldb_ymm5_x86_64:
+  case lldb_ymm6_x86_64:
+  case lldb_ymm7_x86_64:
+  case lldb_ymm8_x86_64:
+  case lldb_ymm9_x86_64:
+  case lldb_ymm10_x86_64:
+  case lldb_ymm11_x86_64:
+  case lldb_ymm12_x86_64:
+  case lldb_ymm13_x86_64:
+  case lldb_ymm14_x86_64:
+  case lldb_ymm15_x86_64:
+#ifdef HAVE_XSTATE
+    if (!(m_xstate_x86_64.xs_rfbm & XCR0_SSE) ||
+        !(m_xstate_x86_64.xs_rfbm & XCR0_YMM_Hi128)) {
+      error.SetErrorStringWithFormat("register \"%s\" not supported by CPU/kernel",
+                                     reg_info->name);
+    } else {
+      uint32_t reg_index = reg - lldb_ymm0_x86_64;
+      YMMReg ymm;
+      ::memcpy(ymm.bytes, reg_value.GetBytes(), reg_value.GetByteSize());
+      YMMToXState(ymm,
+          m_xstate_x86_64.xs_fxsave.fx_xmm[reg_index].xmm_bytes,
+          m_xstate_x86_64.xs_ymm_hi128.xs_ymm[reg_index].ymm_bytes);
+    }
+#else
+    error.SetErrorString("XState not supported by the kernel");
+#endif
+    break;
+  case lldb_dr0_x86_64:
+  case lldb_dr1_x86_64:
+  case lldb_dr2_x86_64:
+  case lldb_dr3_x86_64:
+  case lldb_dr4_x86_64:
+  case lldb_dr5_x86_64:
+  case lldb_dr6_x86_64:
+  case lldb_dr7_x86_64:
+    error.SetErrorString("debug registers not supported by the kernel");
+    break;
+  default:
+    error.SetErrorStringWithFormat("unknown register \"%s\"", reg_info->name);
+  }
+
+  return error;
+}
+
+Status NativeRegisterContextSunOS_x86_64::ReadAllRegisterValues(
+    lldb::DataBufferSP &data_sp) {
+  Status error;
+
+  data_sp.reset(new DataBufferHeap(REG_CONTEXT_SIZE, 0));
+  error = ReadRegisterSet(GPRegSet);
+  if (error.Fail())
+    return error;
+
+  uint8_t *dst = data_sp->GetBytes();
+  ::memcpy(dst, &m_gpr_x86_64, GetRegisterInfoInterface().GetGPRSize());
+  dst += GetRegisterInfoInterface().GetGPRSize();
+
+  RegisterValue value((uint64_t)-1);
+  const RegisterInfo *reg_info =
+      GetRegisterInfoInterface().GetDynamicRegisterInfo("orig_eax");
+  if (reg_info == nullptr)
+    reg_info = GetRegisterInfoInterface().GetDynamicRegisterInfo("orig_rax");
+  return error;
+}
+
+Status NativeRegisterContextSunOS_x86_64::WriteAllRegisterValues(
+    const lldb::DataBufferSP &data_sp) {
+  Status error;
+
+  if (!data_sp) {
+    error.SetErrorStringWithFormat(
+        "NativeRegisterContextSunOS_x86_64::%s invalid data_sp provided",
+        __FUNCTION__);
+    return error;
+  }
+
+  if (data_sp->GetByteSize() != REG_CONTEXT_SIZE) {
+    error.SetErrorStringWithFormat(
+        "NativeRegisterContextSunOS_x86_64::%s data_sp contained mismatched "
+        "data size, expected %" PRIu64 ", actual %" PRIu64,
+        __FUNCTION__, REG_CONTEXT_SIZE, data_sp->GetByteSize());
+    return error;
+  }
+
+  uint8_t *src = data_sp->GetBytes();
+  if (src == nullptr) {
+    error.SetErrorStringWithFormat("NativeRegisterContextSunOS_x86_64::%s "
+                                   "DataBuffer::GetBytes() returned a null "
+                                   "pointer",
+                                   __FUNCTION__);
+    return error;
+  }
+  ::memcpy(&m_gpr_x86_64, src, GetRegisterInfoInterface().GetGPRSize());
+
+  error = WriteRegisterSet(GPRegSet);
+  if (error.Fail())
+    return error;
+  src += GetRegisterInfoInterface().GetGPRSize();
+
+  return error;
+}
+
+uint32_t NativeRegisterContextSunOS_x86_64::NumSupportedHardwareWatchpoints() {
+  // We don't have hardware watchpoings, and I don't see a limit on the number
+  // of watchpoints in procfs. Apparently this is turned into an int somewhere
+  // up the stack, so use INT32_MAX instead of UINT32_MAX.
+  return INT32_MAX;
+}
+
+#endif // defined(__x86_64__)
