$NetBSD$

--- source/Plugins/Process/SunOS/NativeThreadSunOS.cpp.orig	2020-06-04 12:33:49.877719313 +0000
+++ source/Plugins/Process/SunOS/NativeThreadSunOS.cpp
@@ -0,0 +1,175 @@
+//===-- NativeThreadSunOS.cpp -------------------------------- -*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#include "NativeThreadSunOS.h"
+#include "NativeRegisterContextSunOS.h"
+
+#include "NativeProcessSunOS.h"
+
+#include "Plugins/Process/POSIX/CrashReason.h"
+#include "Plugins/Process/POSIX/ProcessPOSIXLog.h"
+#include "lldb/Utility/LLDBAssert.h"
+#include "lldb/Utility/RegisterValue.h"
+#include "lldb/Utility/State.h"
+
+#include <sstream>
+
+#include <libproc.h>
+
+using namespace lldb;
+using namespace lldb_private;
+using namespace lldb_private::process_sunos;
+
+NativeThreadSunOS::NativeThreadSunOS(NativeProcessSunOS &process,
+                                       lldb::tid_t tid)
+    : NativeThreadProtocol(process, tid), m_state(StateType::eStateInvalid),
+      m_stop_info(), m_reg_context_up(
+NativeRegisterContextSunOS::CreateHostNativeRegisterContextSunOS(process.GetArchitecture(), *this)
+), m_stop_description() {
+  int lerr;
+
+  m_lwp = Lgrab(process.GetProcHdl(), tid, &lerr);
+
+  if (m_lwp == NULL) {
+    Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_THREAD));
+    LLDB_LOG(log, "failed to grab thread with tid = {0} with error: {1}",
+             tid, Lgrab_error(lerr));
+  }
+}
+
+NativeThreadSunOS::~NativeThreadSunOS() {
+  Lfree(m_lwp);
+}
+
+void NativeThreadSunOS::SetStoppedBySignal(uint32_t signo,
+                                            const siginfo_t *info) {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_THREAD));
+  LLDB_LOG(log, "tid = {0} in called with signal {1}", GetID(), signo);
+
+  SetStopped();
+
+  m_stop_info.reason = StopReason::eStopReasonSignal;
+  m_stop_info.details.signal.signo = signo;
+
+  m_stop_description.clear();
+  if (info) {
+    switch (signo) {
+    case SIGSEGV:
+    case SIGBUS:
+    case SIGFPE:
+    case SIGILL:
+      const auto reason = GetCrashReason(*info);
+      m_stop_description = GetCrashReasonString(reason, *info);
+      break;
+    }
+  }
+}
+
+void NativeThreadSunOS::SetStoppedByBreakpoint() {
+  SetStopped();
+  m_stop_info.reason = StopReason::eStopReasonBreakpoint;
+}
+
+void NativeThreadSunOS::SetStoppedByTrace() {
+  SetStopped();
+  m_stop_info.reason = StopReason::eStopReasonTrace;
+}
+
+void NativeThreadSunOS::SetStoppedByExec() {
+  SetStopped();
+  m_stop_info.reason = StopReason::eStopReasonExec;
+}
+
+void NativeThreadSunOS::SetStoppedByWatchpoint(lldb::addr_t wpaddr,
+                                               lldb::addr_t addr) {
+  SetStopped();
+  lldbassert(wpaddr != LLDB_INVALID_ADDRESS && "wpaddr cannot be invalid");
+
+  std::ostringstream ostr;
+  ostr << wpaddr << " ";
+  ostr << LLDB_INVALID_INDEX32; // we don't have WP indices
+  ostr << " " << addr;
+
+  m_stop_description = ostr.str();
+  m_stop_info.reason = StopReason::eStopReasonWatchpoint;
+}
+
+void NativeThreadSunOS::SetStopped() {
+  const StateType new_state = StateType::eStateStopped;
+  m_state = new_state;
+  m_stop_description.clear();
+  m_stop_info.reason = StopReason::eStopReasonNone;
+}
+
+void NativeThreadSunOS::SetRunning() {
+  m_state = StateType::eStateRunning;
+  m_stop_info.reason = StopReason::eStopReasonNone;
+}
+
+void NativeThreadSunOS::SetStepping() {
+  m_state = StateType::eStateStepping;
+  m_stop_info.reason = StopReason::eStopReasonNone;
+}
+
+std::string NativeThreadSunOS::GetName() { return std::string(""); }
+
+lldb::StateType NativeThreadSunOS::GetState() { return m_state; }
+
+bool NativeThreadSunOS::GetStopReason(ThreadStopInfo &stop_info,
+                                       std::string &description) {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_THREAD));
+
+  description.clear();
+
+  switch (m_state) {
+  case eStateStopped:
+  case eStateCrashed:
+  case eStateExited:
+  case eStateSuspended:
+  case eStateUnloaded:
+    stop_info = m_stop_info;
+    description = m_stop_description;
+
+    return true;
+
+  case eStateInvalid:
+  case eStateConnected:
+  case eStateAttaching:
+  case eStateLaunching:
+  case eStateRunning:
+  case eStateStepping:
+  case eStateDetached:
+    LLDB_LOG(log, "tid = {0} in state {1} cannot answer stop reason", GetID(),
+             StateAsCString(m_state));
+    return false;
+  }
+  llvm_unreachable("unhandled StateType!");
+}
+
+NativeRegisterContext& NativeThreadSunOS::GetRegisterContext() {
+  assert(m_reg_context_up);
+return  *m_reg_context_up;
+}
+
+Status NativeThreadSunOS::SetWatchpoint(lldb::addr_t addr, size_t size,
+                                         uint32_t watch_flags, bool hardware) {
+  return Status("not implemented");
+}
+
+Status NativeThreadSunOS::RemoveWatchpoint(lldb::addr_t addr) {
+  return Status("not implemented");
+}
+
+Status NativeThreadSunOS::SetHardwareBreakpoint(lldb::addr_t addr,
+                                                 size_t size) {
+  return Status("not implemented");
+}
+
+Status NativeThreadSunOS::RemoveHardwareBreakpoint(lldb::addr_t addr) {
+  return Status("not implemented");
+}
