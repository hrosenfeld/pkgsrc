$NetBSD$

--- source/Plugins/Process/SunOS/NativeProcessSunOS.cpp.orig	2020-07-08 16:05:07.696960324 +0000
+++ source/Plugins/Process/SunOS/NativeProcessSunOS.cpp
@@ -0,0 +1,1129 @@
+//===-- NativeProcessSunOS.cpp ------------------------------- -*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#include "NativeProcessSunOS.h"
+
+
+
+#include "Plugins/Process/POSIX/ProcessPOSIXLog.h"
+#include "lldb/Host/sunos/HostProcessSunOS.h"
+#include "lldb/Host/common/NativeRegisterContext.h"
+#include "lldb/Host/posix/ProcessLauncherPosixFork.h"
+#include "lldb/Target/Process.h"
+#include "lldb/Utility/State.h"
+#include "llvm/Support/Errno.h"
+
+// System includes - They have to be included after framework includes because
+// they define some macros which collide with variable names in other modules
+// clang-format off
+#include <sys/types.h>
+#include <sys/wait.h>
+#include <elf.h>
+#include <libproc.h>
+#include <siginfo.h>
+// clang-format on
+
+using namespace lldb;
+using namespace lldb_private;
+using namespace lldb_private::process_sunos;
+using namespace llvm;
+
+// Simple helper function to ensure flags are enabled on the given file
+// descriptor.
+static Status EnsureFDFlags(int fd, int flags) {
+  Status error;
+
+  int status = fcntl(fd, F_GETFL);
+  if (status == -1) {
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  if (fcntl(fd, F_SETFL, status | flags) == -1) {
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  return error;
+}
+
+static void PostAttachFlags(struct ps_prochandle *pr)
+{
+  // Trace breakpoints, watchpoints, and single-step.
+  (void) Pfault(pr, FLTBPT, 1);
+  (void) Pfault(pr, FLTWATCH, 1);
+  (void) Pfault(pr, FLTTRACE, 1);
+
+  // Use synchronous mode so that all threads are stopped at the same time.
+  (void) Punsetflags(pr, PR_ASYNC);
+
+  // Inherit tracing on fork.
+  (void) Psetflags(pr, PR_FORK);
+
+  // Automatically adjust the PC on breakpoints
+  (void) Psetflags(pr, PR_BPTADJ);
+
+  // Add sysexits - vfork, forksys, execve?
+}
+
+// Public Static Methods
+
+llvm::Expected<std::unique_ptr<NativeProcessProtocol>>
+NativeProcessSunOS::Factory::Launch(ProcessLaunchInfo &launch_info,
+                                     NativeDelegate &native_delegate,
+                                     MainLoop &mainloop) const {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+
+  Status status;
+  HostProcess proc =
+    ProcessLauncherPosixFork().LaunchProcess(launch_info, status);
+  if (status.Fail()) {
+    LLDB_LOG(log, "failed to launch process: {0}", status);
+    return status.ToError();
+  }
+
+  ::pid_t pid = proc.GetProcessId();
+  LLDB_LOG(log, "pid = {0:x}", pid);
+
+  ProcessInstanceInfo Info;
+  if (!Host::GetProcessInfo(pid, Info)) {
+    return llvm::make_error<StringError>("Cannot get process architecture",
+                                         llvm::inconvertibleErrorCode());
+  }
+
+  // Set the architecture to the exe architecture.
+  LLDB_LOG(log, "pid = {0:x}, detected architecture {1}", pid,
+           Info.GetArchitecture().GetArchitectureName());
+
+  HostProcessSunOS *proc_s =
+    static_cast<HostProcessSunOS *>(&proc.GetNativeProcess());
+  struct ps_prochandle *pr = proc_s->GetProcHdl();
+
+  // Make sure this process doesn't run on last close by the debugger but
+  // is killed instead.
+  (void) Punsetflags(pr, PR_RLC);
+  (void) Psetflags(pr, PR_KLC);
+
+  PostAttachFlags(pr);
+
+  std::unique_ptr<NativeProcessSunOS> process_up(new NativeProcessSunOS(
+      pid, launch_info.GetPTY().ReleaseMasterFileDescriptor(), native_delegate,
+      Info.GetArchitecture(), mainloop, pr));
+
+  status = process_up->ReinitializeThreads();
+  if (status.Fail())
+    return status.ToError();
+
+  for (const auto &thread : process_up->m_threads)
+    static_cast<NativeThreadSunOS &>(*thread).SetStoppedByTrace();
+  process_up->SetState(StateType::eStateStopped, false);
+
+  LLDB_LOG(log, "inferior started, now in stopped state");
+
+  return std::move(process_up);
+}
+
+llvm::Expected<std::unique_ptr<NativeProcessProtocol>>
+NativeProcessSunOS::Factory::Attach(
+    lldb::pid_t pid, NativeProcessProtocol::NativeDelegate &native_delegate,
+    MainLoop &mainloop) const {
+  Status status;
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  LLDB_LOG(log, "pid = {0:x}", pid);
+
+  // Retrieve the architecture for the running process.
+  ProcessInstanceInfo Info;
+  if (!Host::GetProcessInfo(pid, Info)) {
+    return llvm::make_error<StringError>("Cannot get process architecture",
+                                         llvm::inconvertibleErrorCode());
+  }
+
+  // Set the architecture to the exe architecture.
+  LLDB_LOG(log, "pid = {0:x}, detected architecture {1}", pid,
+           Info.GetArchitecture().GetArchitectureName());
+
+
+  int err;
+  struct ps_prochandle *pr = Pgrab(pid, 0, &err);
+  if (pr == NULL) {
+    status.SetErrorStringWithFormat("Pgrab: %s", Pgrab_error(err));
+    return status.ToError();
+  }
+
+  PostAttachFlags(pr);
+
+  std::unique_ptr<NativeProcessSunOS> process_up(new NativeProcessSunOS(
+      pid, -1, native_delegate, Info.GetArchitecture(), mainloop, pr));
+
+  status = process_up->ReinitializeThreads();
+  if (status.Fail())
+    return status.ToError();
+
+  for (const auto &thread : process_up->m_threads)
+    static_cast<NativeThreadSunOS &>(*thread).SetStoppedByTrace();
+  process_up->SetState(StateType::eStateStopped, false);
+
+  return std::move(process_up);
+}
+
+// Public Instance Methods
+
+NativeProcessSunOS::NativeProcessSunOS(::pid_t pid,
+                                       int terminal_fd,
+                                       NativeDelegate &delegate,
+                                       const ArchSpec &arch,
+                                       MainLoop &mainloop,
+                                       struct ps_prochandle *pr)
+    : NativeProcessProtocol(pid, terminal_fd, delegate), m_arch(arch),
+      m_mainloop(mainloop), m_pr(pr), m_pwait_pipe(), m_pwait_thread(0),
+      m_pwait_reader_handle(), m_last_watchpoint(nullptr) {
+
+  if (m_terminal_fd != -1) {
+    Status status = EnsureFDFlags(m_terminal_fd, O_NONBLOCK);
+    assert(status.Success());
+  }
+
+  Status status;
+  m_sigchld_handle = m_mainloop.RegisterSignal(
+      SIGCHLD, [this](MainLoopBase &) { SigchldHandler(); }, status);
+  assert(m_sigchld_handle && status.Success());
+}
+
+Status NativeProcessSunOS::StartPwaitThread() {
+  Status error;
+  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PROCESS));
+
+  // Every time we set a process running we'll create a thread that runs Pwait()
+  // to wait for the debugged process to stop. When Pwait() returns we'll send a
+  // byte over a pipe that the main loop is monitoring, and then we'll exit.
+
+  const bool child_inherits = false;
+  error = m_pwait_pipe.CreateNew(child_inherits);
+  if (error.Fail()) {
+    LLDB_LOG(log, "failed to create Pwait communication pipe: {0}", error.AsCString());
+    return error;
+  }
+
+  // TODO make PipePOSIX derive from IOObject.  This is goofy here.
+  const bool transfer_ownership = false;
+  auto io_sp = IOObjectSP(
+      new File(m_pwait_pipe.GetReadFileDescriptor(), transfer_ownership));
+  m_pwait_reader_handle = m_mainloop.RegisterReadObject(
+      io_sp, [this](MainLoopBase &) { HandlePwaitResult(); }, error);
+
+  // Create the thread.
+  auto pthread_err =
+      ::pthread_create(&m_pwait_thread, nullptr, PwaitThread, this);
+  error.SetError(pthread_err, eErrorTypePOSIX);
+  if (error.Fail()) {
+    LLDB_LOG(log, "failed to create Pwait thread: {0} ({1})",
+                  error.GetError(), error.AsCString());
+    return error;
+  }
+
+  return error;
+}
+
+void *NativeProcessSunOS::PwaitThread(void *arg) {
+  return static_cast<NativeProcessSunOS *>(arg)->PwaitThread();
+}
+
+
+void *NativeProcessSunOS::PwaitThread() {
+  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PROCESS));
+  Status error;
+
+  if (m_pr == NULL) {
+    LLDB_LOG(log, "no process handle");
+    return nullptr;
+  }
+
+  // Name the thread.
+  pthread_setname_np(pthread_self(), "Pwait thread");
+
+  for (;;) {
+    if (Pwait(m_pr, 0) == 0) {
+      size_t bytes_written = 0;
+      char p = 'p';
+      error = m_pwait_pipe.Write(&p, 1, bytes_written);
+      if (error.Fail() || (bytes_written < 1))
+        LLDB_LOG(log, "failed to write Pwait pipe: {0}", error.AsCString());
+      m_pwait_pipe.CloseWriteFileDescriptor();
+      break;
+    }
+
+    if (errno == EINTR)
+      continue;
+    if (Pstate(m_pr) == PS_LOST)
+      if (Preopen(m_pr) == 0)
+        continue;
+    error.SetErrorToErrno();
+    LLDB_LOG(log, "Pwait failed: {0}", error.AsCString());
+    break;
+  }
+
+  return nullptr;
+}
+
+Status NativeProcessSunOS::HandlePwaitResult() {
+  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PROCESS));
+  Status error;
+
+  // Read from the pipe.
+  size_t bytes_read = 0;
+  char p = '\0';
+
+  error = m_pwait_pipe.Read(&p, 1, bytes_read);
+  m_pwait_pipe.CloseReadFileDescriptor();
+  if (error.Fail() || (bytes_read < 1)) {
+    LLDB_LOG(log, "failed to read Pwait pipe: {0}", error.AsCString());
+    return error;
+  }
+  
+  m_pwait_reader_handle.reset();
+
+  // Update the thread list. This should really use the notification mechanism
+  // of libc_db, but doing it this way is much less complicated.
+  error = ReinitializeThreads();
+  if (error.Fail()) {
+    SetState(StateType::eStateInvalid);
+    return error;
+  }
+
+  // Update the state of each thread.
+  for (const auto &t : m_threads) {
+    NativeThreadSunOS &thread = static_cast<NativeThreadSunOS &>(*t);
+    const lwpstatus_t *lps = Lstatus(thread.GetLwpHdl());
+
+    switch (lps->pr_why) {
+    case PR_SIGNALLED:
+      thread.SetStoppedBySignal(lps->pr_what, &lps->pr_info);
+      break;
+    case PR_SYSEXIT:
+        thread.SetStoppedByExec();
+      break;
+    case PR_FAULTED:
+      switch (lps->pr_what) {
+      case FLTTRACE:
+        thread.SetStoppedByTrace();
+        break;
+      case FLTBPT:
+        thread.SetStoppedByBreakpoint();
+        break;
+      case FLTWATCH: {
+        lldb::addr_t addr = (uintptr_t)lps->pr_info.si_addr;
+        if (m_last_watchpoint == nullptr)
+          m_last_watchpoint = GetSoftwareWatchpoint(addr);
+        thread.SetStoppedByWatchpoint(m_last_watchpoint->pr_vaddr, addr);
+        break;
+      }
+      default:
+        LLDB_LOG(log, "unexpected pr_what {0} for pr_why = PR_FAULTED",
+                 lps->pr_what);
+        thread.SetStoppedByTrace();
+      }
+      break;
+    default:
+      LLDB_LOG(log, "unexpected pr_why {0}", lps->pr_why);
+    case PR_REQUESTED:
+      thread.SetStopped();
+    }
+  }
+
+  SetState(eStateStopped, true);
+
+  return error;
+}
+
+Status NativeProcessSunOS::SetSoftwareBreakpoint(lldb::addr_t addr,
+                                                 uint32_t size_hint) {
+  Log *log(GetLogIfAnyCategoriesSet(LIBLLDB_LOG_BREAKPOINTS));
+  LLDB_LOG(log, "addr = {0:x}, size_hint = {1}", addr, size_hint);
+
+  auto it = m_software_breakpoints.find(addr);
+  if (it != m_software_breakpoints.end()) {
+    ++it->second.ref_count;
+    return Status();
+  }
+  auto expected_bkpt = EnableSoftwareBreakpoint(addr, size_hint);
+  if (!expected_bkpt)
+    return Status(expected_bkpt.takeError());
+
+  m_software_breakpoints.emplace(addr, std::move(*expected_bkpt));
+  return Status();
+}
+
+Status NativeProcessSunOS::RemoveSoftwareBreakpoint(lldb::addr_t addr) {
+  Log *log(GetLogIfAnyCategoriesSet(LIBLLDB_LOG_BREAKPOINTS));
+  LLDB_LOG(log, "addr = {0:x}", addr);
+
+  auto it = m_software_breakpoints.find(addr);
+  if (it == m_software_breakpoints.end())
+    return Status("Breakpoint not found.");
+  assert(it->second.ref_count > 0);
+  if (--it->second.ref_count > 0)
+    return Status();
+
+  ulong_t saved = 0;
+  memcpy(&saved, it->second.saved_opcodes.data(),
+         it->second.saved_opcodes.size());
+  if (Pdelbkpt(m_pr, addr, saved) == -1) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  m_software_breakpoints.erase(it);
+  return Status();
+}
+
+llvm::Expected<NativeProcessProtocol::SoftwareBreakpoint>
+NativeProcessSunOS::EnableSoftwareBreakpoint(lldb::addr_t addr, uint32_t size_hint) {
+  Log *log(GetLogIfAnyCategoriesSet(LIBLLDB_LOG_BREAKPOINTS));
+  llvm::SmallVector<uint8_t, 4> saved_opcode_bytes(4, 0);
+  ulong_t saved = 0;
+
+  if (Psetbkpt(m_pr, addr, &saved) == -1) {
+    Status error;
+    error.SetErrorToErrno();
+    return error.ToError();
+  }
+
+  memcpy(saved_opcode_bytes.data(), &saved, saved_opcode_bytes.size());
+
+  LLDB_LOG(log, "Overwrote bytes at {0:x}: {1:@[x]}", addr,
+      llvm::make_range(saved_opcode_bytes.begin(), saved_opcode_bytes.end()));
+
+  return SoftwareBreakpoint{1, saved_opcode_bytes, 0};
+}
+
+llvm::Expected<llvm::ArrayRef<uint8_t>>
+NativeProcessSunOS::GetSoftwareBreakpointTrapOpcode(size_t size_hint) {
+  return llvm::createStringError(llvm::inconvertibleErrorCode(),
+    "software breakpoints on SunOS only supported through libproc interface");
+}
+
+prwatch_t *NativeProcessSunOS::GetSoftwareWatchpoint(lldb::addr_t addr) {
+  Log *log(GetLogIfAnyCategoriesSet(LIBLLDB_LOG_WATCHPOINTS));
+  LLDB_LOG(log, "looking up watchpoint matching addr = {0:x}", addr);
+
+  for (auto it = m_software_watchpoints.begin();
+       it != m_software_watchpoints.end();
+       ++it) {
+    prwatch_t *prw = &it->second.prw;
+    if (addr >= prw->pr_vaddr && addr < prw->pr_vaddr + prw->pr_size)
+      return (prw);
+  }
+
+  return nullptr;
+}
+
+Status NativeProcessSunOS::SetSoftwareWatchpoint(lldb::addr_t addr, size_t size,
+                                                 uint32_t flags) {
+  Log *log(GetLogIfAnyCategoriesSet(LIBLLDB_LOG_WATCHPOINTS));
+  LLDB_LOG(log, "addr = {0:x}, size = {1}, flags = {2}", addr, size, flags);
+
+  auto it = m_software_watchpoints.find(addr);
+  if (it != m_software_watchpoints.end()) {
+    ++it->second.ref_count;
+    return Status();
+  }
+  auto expected_wapt = EnableSoftwareWatchpoint(addr, size, flags);
+  if (!expected_wapt)
+    return Status(expected_wapt.takeError());
+
+  m_software_watchpoints.emplace(addr, std::move(*expected_wapt));
+  return Status();
+}
+
+Status NativeProcessSunOS::RemoveSoftwareWatchpoint(lldb::addr_t addr) {
+  Log *log(GetLogIfAnyCategoriesSet(LIBLLDB_LOG_WATCHPOINTS));
+  LLDB_LOG(log, "addr = {0:x}", addr);
+
+  auto it = m_software_watchpoints.find(addr);
+  if (it == m_software_watchpoints.end())
+    return Status("Watchpoint not found.");
+  assert(it->second.ref_count > 0);
+  if (--it->second.ref_count > 0)
+    return Status();
+
+  if (Pdelwapt(m_pr, &it->second.prw) == -1) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  m_software_watchpoints.erase(it);
+  return Status();
+}
+
+llvm::Expected<NativeProcessSunOS::SoftwareWatchpoint>
+NativeProcessSunOS::EnableSoftwareWatchpoint(lldb::addr_t addr, size_t size,
+                                             uint32_t flags) {
+  int wflags = 0;
+
+  if ((flags & 1) != 0)
+    wflags |= WA_WRITE;
+  if ((flags & 2) != 0)
+    wflags |= WA_READ;
+
+  prwatch_t prw = {addr, size, wflags};
+
+  if (Psetwapt(m_pr, &prw) == -1) {
+    Status error;
+    error.SetErrorToErrno();
+    return error.ToError();
+  }
+
+  return SoftwareWatchpoint{1, prw};
+}
+
+
+// Handles all waitpid events from the inferior process.
+void NativeProcessSunOS::MonitorCallback(lldb::pid_t pid, int signal) {
+  switch (signal) {
+  case SIGTRAP:
+    return MonitorSIGTRAP(pid);
+  case SIGSTOP:
+    return MonitorSIGSTOP(pid);
+  default:
+    return MonitorSignal(pid, signal);
+  }
+}
+
+void NativeProcessSunOS::MonitorExited(lldb::pid_t pid, WaitStatus status) {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+
+  LLDB_LOG(log, "got exit signal({0}) , pid = {1}", status, pid);
+  
+  /* Stop Tracking All Threads attached to Process */
+  m_threads.clear();
+
+  SetExitStatus(status, true);
+
+  // Notify delegate that our process has exited.
+  SetState(StateType::eStateExited, true);
+}
+
+void NativeProcessSunOS::MonitorSIGSTOP(lldb::pid_t pid) {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  LLDB_LOG(log, "got SIGSTOP");
+  assert(m_pr != NULL);
+
+  const pstatus_t *ps = Pstatus(m_pr);
+
+  // Handle SIGSTOP from LLGS (LLDB GDB Server)
+  if (ps->pr_lwp.pr_info.si_code == SI_USER &&
+      ps->pr_lwp.pr_info.si_pid == ::getpid()) {
+    /* Stop Tracking all Threads attached to Process */
+    for (const auto &thread : m_threads) {
+      static_cast<NativeThreadSunOS &>(*thread).SetStoppedBySignal(
+        SIGSTOP, &ps->pr_lwp.pr_info);
+    }
+  }
+}
+
+void NativeProcessSunOS::MonitorSIGTRAP(lldb::pid_t pid) {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  LLDB_LOG(log, "got SIGTRAP");
+  assert(m_pr != NULL);
+
+  const pstatus_t *ps = Pstatus(m_pr);
+
+  switch (ps->pr_lwp.pr_info.si_code) {
+  case TRAP_BRKPT:
+    LLDB_LOG(log, "TRAP_BRKPT, shouldn't happen");
+    assert(0);
+    for (const auto &thread : m_threads)
+      static_cast<NativeThreadSunOS &>(*thread).SetStoppedByBreakpoint();
+    SetState(StateType::eStateStopped, true);
+    break;
+  case TRAP_TRACE:
+    LLDB_LOG(log, "TRAP_TRACE, shouldn't happen");
+    assert(0);
+    for (const auto &thread : m_threads)
+      static_cast<NativeThreadSunOS &>(*thread).SetStoppedByTrace();
+    SetState(StateType::eStateStopped, true);
+    break;
+  }
+}
+
+void NativeProcessSunOS::MonitorSignal(lldb::pid_t pid, int signal) {
+  assert(m_pr != NULL);
+
+  const pstatus_t *ps = Pstatus(m_pr);
+
+  for (const auto &thread : m_threads) {
+    static_cast<NativeThreadSunOS &>(*thread).SetStoppedBySignal(
+        ps->pr_lwp.pr_info.si_signo, &ps->pr_lwp.pr_info);
+  }
+  SetState(StateType::eStateStopped, true);
+}
+
+Status NativeProcessSunOS::Resume(const ResumeActionList &resume_actions) {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  LLDB_LOG(log, "pid {0}", GetID());
+
+  assert(m_pr != NULL);
+
+  lldb::StateType state = eStateStopped;
+
+  for (const auto &t : m_threads) {
+    NativeThreadSunOS &thread = static_cast<NativeThreadSunOS &>(*t);
+    const ResumeAction *const action =
+      resume_actions.GetActionForThread(thread.GetID(), true);
+
+    if (action == nullptr) {
+      LLDB_LOG(log, "no action specified for pid {0} tid {1}", GetID(),
+               thread.GetID());
+      continue;
+    } else {
+      LLDB_LOG(log, "pid {0} tid {1}: action {2}, signal {3}",
+               GetID(), thread.GetID(), action->state, action->signal);
+    }
+
+    Status error;
+
+    switch (action->state) {
+    case eStateRunning:
+      // If we're stopped on a watchpoint, step over it first.
+      if (m_last_watchpoint != nullptr) {
+        if (Lxecwapt(thread.GetLwpHdl(), m_last_watchpoint) != 0) {
+          error.SetErrorToErrno();
+          return error;
+        }
+        m_last_watchpoint = nullptr;
+      }
+
+      // Run the thread, possibly feeding it the signal.
+      if (Lsetrun(thread.GetLwpHdl(), action->signal, PRCFAULT) != 0) {
+        error.SetErrorToErrno();
+        return error;
+      }
+
+      thread.SetRunning();
+
+      if (state != eStateStopped && state != eStateRunning)
+        LLDB_LOG(log, "inconsistent state - thread {0} set running, but "
+                 "process is in state {1}", thread.GetID(), state);
+
+      state = eStateRunning;
+      break;
+
+    case eStateStepping:
+      // If we're stopped on a watchpoint, step over it.
+      if (m_last_watchpoint != nullptr) {
+        if (Lxecwapt(thread.GetLwpHdl(), m_last_watchpoint) != 0) {
+          error.SetErrorToErrno();
+          return error;
+        }
+        m_last_watchpoint = nullptr;
+      } else {
+        // Step the thread, possibly feeding it the signal.
+        if (Lsetrun(thread.GetLwpHdl(), action->signal, PRSTEP | PRCFAULT) != 0) {
+          error.SetErrorToErrno();
+          return error;
+        }
+      }
+
+      thread.SetStepping();
+
+      if (state != eStateStopped && state != eStateStepping)
+        LLDB_LOG(log, "inconsistent state - thread {0} set stepping, but "
+                 "process is in state {1}", thread.GetID(), state);
+
+      state = eStateStepping;
+      break;
+
+    case eStateSuspended:
+    case eStateStopped:
+      break;
+
+    default:
+      return Status("NativeProcessSunOS::%s (): unexpected state %s specified "
+                    "for pid %" PRIu64 ", tid %" PRIu64,
+                    __FUNCTION__, StateAsCString(action->state), GetID(),
+                    thread.GetID());
+    }
+  }
+
+  if (state != eStateStopped) {
+    SetState(state, true);
+    StartPwaitThread();
+  }
+
+  return Status();
+}
+
+Status NativeProcessSunOS::Halt() {
+  Status error;
+
+  if (kill(GetID(), SIGSTOP) != 0)
+    error.SetErrorToErrno();
+
+  return error;
+}
+
+Status NativeProcessSunOS::Detach() {
+  Status error;
+
+  assert(m_pr != NULL);
+
+  // Stop monitoring the inferior.
+  m_sigchld_handle.reset();
+
+  Prelease(m_pr, 0);
+  m_pr = NULL;
+
+  return error;
+}
+
+Status NativeProcessSunOS::Signal(int signo) {
+  Status error;
+
+  if (kill(GetID(), signo))
+    error.SetErrorToErrno();
+
+  return error;
+}
+
+Status NativeProcessSunOS::Interrupt() {
+  Status error;
+
+  if (Pstop(m_pr, 0) == 0) {
+    for (const auto &thread : m_threads) {
+      static_cast<NativeThreadSunOS &>(*thread).SetStoppedBySignal(SIGSTOP);
+    }
+    SetState(StateType::eStateStopped, true);
+  } else {
+    error.SetErrorToErrno();
+  }
+
+  return error;
+}
+
+Status NativeProcessSunOS::Kill() {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  LLDB_LOG(log, "pid {0}", GetID());
+
+  Status error;
+
+  switch (m_state) {
+  case StateType::eStateInvalid:
+  case StateType::eStateExited:
+  case StateType::eStateCrashed:
+  case StateType::eStateDetached:
+  case StateType::eStateUnloaded:
+    // Nothing to do - the process is already dead.
+    LLDB_LOG(log, "ignored for PID {0} due to current state: {1}", GetID(),
+             StateAsCString(m_state));
+    return error;
+
+  case StateType::eStateConnected:
+  case StateType::eStateAttaching:
+  case StateType::eStateLaunching:
+  case StateType::eStateStopped:
+  case StateType::eStateRunning:
+  case StateType::eStateStepping:
+  case StateType::eStateSuspended:
+    // We can try to kill a process in these states.
+    break;
+  }
+
+  if (kill(GetID(), SIGKILL) != 0) {
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  return error;
+}
+
+Status NativeProcessSunOS::GetMemoryRegionInfo(lldb::addr_t load_addr,
+                                                MemoryRegionInfo &range_info) {
+
+  if (m_supports_mem_region == LazyBool::eLazyBoolNo) {
+    // We're done.
+    return Status("unsupported");
+  }
+
+  Status error = PopulateMemoryRegionCache();
+  if (error.Fail()) {
+    return error;
+  }
+
+  lldb::addr_t prev_base_address = 0;
+  // FIXME start by finding the last region that is <= target address using
+  // binary search.  Data is sorted.
+  // There can be a ton of regions on pthreads apps with lots of threads.
+  for (auto it = m_mem_region_cache.begin(); it != m_mem_region_cache.end();
+       ++it) {
+    MemoryRegionInfo &proc_entry_info = it->first;
+    // Sanity check assumption that memory map entries are ascending.
+    assert((proc_entry_info.GetRange().GetRangeBase() >= prev_base_address) &&
+           "descending memory map entries detected, unexpected");
+    prev_base_address = proc_entry_info.GetRange().GetRangeBase();
+    UNUSED_IF_ASSERT_DISABLED(prev_base_address);
+    // If the target address comes before this entry, indicate distance to next
+    // region.
+    if (load_addr < proc_entry_info.GetRange().GetRangeBase()) {
+      range_info.GetRange().SetRangeBase(load_addr);
+      range_info.GetRange().SetByteSize(
+          proc_entry_info.GetRange().GetRangeBase() - load_addr);
+      range_info.SetReadable(MemoryRegionInfo::OptionalBool::eNo);
+      range_info.SetWritable(MemoryRegionInfo::OptionalBool::eNo);
+      range_info.SetExecutable(MemoryRegionInfo::OptionalBool::eNo);
+      range_info.SetMapped(MemoryRegionInfo::OptionalBool::eNo);
+      return error;
+    } else if (proc_entry_info.GetRange().Contains(load_addr)) {
+      // The target address is within the memory region we're processing here.
+      range_info = proc_entry_info;
+      return error;
+    }
+    // The target memory address comes somewhere after the region we just
+    // parsed.
+  }
+  // If we made it here, we didn't find an entry that contained the given
+  // address. Return the load_addr as start and the amount of bytes betwwen
+  // load address and the end of the memory as size.
+  range_info.GetRange().SetRangeBase(load_addr);
+  range_info.GetRange().SetRangeEnd(LLDB_INVALID_ADDRESS);
+  range_info.SetReadable(MemoryRegionInfo::OptionalBool::eNo);
+  range_info.SetWritable(MemoryRegionInfo::OptionalBool::eNo);
+  range_info.SetExecutable(MemoryRegionInfo::OptionalBool::eNo);
+  range_info.SetMapped(MemoryRegionInfo::OptionalBool::eNo);
+  return error;
+}
+
+int NativeProcessSunOS::MapIterCB(void *arg, const prmap_t *prmap, const char *name) {
+  NativeProcessSunOS *P = (NativeProcessSunOS *)arg;
+
+  return P->PopulateOneMemoryRegion(prmap, name);
+}
+
+Status NativeProcessSunOS::PopulateMemoryRegionCache() {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  // If our cache is empty, pull the latest.  There should always be at least
+  // one memory region if memory region handling is supported.
+  if (!m_mem_region_cache.empty()) {
+    LLDB_LOG(log, "reusing {0} cached memory region entries",
+             m_mem_region_cache.size());
+    return Status();
+  }
+
+  assert(m_pr != NULL);
+
+  if (Pmapping_iter_resolved(m_pr, MapIterCB, this) == -1) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  if (m_mem_region_cache.empty()) {
+    // No entries after attempting to read them.  This shouldn't happen. Assume
+    // we don't support map entries.
+    LLDB_LOG(log, "failed to find any vmmap entries, assuming no support "
+                  "for memory region metadata retrieval");
+    m_supports_mem_region = LazyBool::eLazyBoolNo;
+    Status error;
+    error.SetErrorString("not supported");
+    return error;
+  }
+  LLDB_LOG(log, "read {0} memory region entries from process {1}",
+           m_mem_region_cache.size(), GetID());
+  // We support memory retrieval, remember that.
+  m_supports_mem_region = LazyBool::eLazyBoolYes;
+  return Status();
+}
+
+int NativeProcessSunOS::PopulateOneMemoryRegion(const prmap_t *prmap,
+                                                const char *name) {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  MemoryRegionInfo info;
+
+  info.Clear();
+  info.GetRange().SetRangeBase(prmap->pr_vaddr);
+  info.GetRange().SetByteSize(prmap->pr_size);
+  info.SetMapped(MemoryRegionInfo::OptionalBool::eYes);
+  
+  if ((prmap->pr_mflags & MA_READ) != 0)
+    info.SetReadable(MemoryRegionInfo::OptionalBool::eYes);
+  else
+    info.SetReadable(MemoryRegionInfo::OptionalBool::eNo);
+
+  if ((prmap->pr_mflags & MA_WRITE) != 0)
+    info.SetWritable(MemoryRegionInfo::OptionalBool::eYes);
+  else
+    info.SetWritable(MemoryRegionInfo::OptionalBool::eNo);
+
+  if ((prmap->pr_mflags & MA_EXEC) != 0)
+    info.SetExecutable(MemoryRegionInfo::OptionalBool::eYes);
+  else
+    info.SetExecutable(MemoryRegionInfo::OptionalBool::eNo);
+
+  if (name != NULL) {
+    if (name[0] == '/') {
+      info.SetName(name);
+    } else {
+      // For some reason libproc() was unable to resolve the name for us.
+      // Use pr_mapname, which is the name under /proc/<pid>/object.
+      char objpath[PATH_MAX] = "";
+      snprintf(objpath, PATH_MAX, "/proc/%" PRIu64 "/object/%s", GetID(),
+               prmap->pr_mapname);
+      info.SetName(objpath);
+    }
+  }
+
+  LLDB_LOG(log,
+           "memory region: {0}, {1}:{2}",
+           name, prmap->pr_vaddr, prmap->pr_size);
+
+  m_mem_region_cache.emplace_back(info, FileSpec(info.GetName().GetCString()));
+
+  return (0);
+}
+
+Status NativeProcessSunOS::AllocateMemory(size_t size, uint32_t permissions,
+                                           lldb::addr_t &addr) {
+  return Status("Unimplemented");
+}
+
+Status NativeProcessSunOS::DeallocateMemory(lldb::addr_t addr) {
+  return Status("Unimplemented");
+}
+
+lldb::addr_t NativeProcessSunOS::GetSharedLibraryInfoAddress() {
+  // punt on this for now
+  return LLDB_INVALID_ADDRESS;
+}
+
+size_t NativeProcessSunOS::UpdateThreads() { return m_threads.size(); }
+
+Status NativeProcessSunOS::SetBreakpoint(lldb::addr_t addr, uint32_t size,
+                                          bool hardware) {
+  if (hardware)
+    return Status("NativeProcessSunOS does not support hardware breakpoints");
+  else
+    return SetSoftwareBreakpoint(addr, size);
+}
+
+Status NativeProcessSunOS::SetWatchpoint(lldb::addr_t addr, size_t size,
+                                         uint32_t watch_flags, bool hardware) {
+  // Apparently LLDB currently only uses "hardware" watchpoints.
+  // We don't really care.
+  return SetSoftwareWatchpoint(addr, size, watch_flags);
+}
+
+Status NativeProcessSunOS::RemoveWatchpoint(lldb::addr_t addr) {
+    return RemoveSoftwareWatchpoint(addr);
+}
+
+Status NativeProcessSunOS::GetLoadedModuleFileSpec(const char *module_path,
+                                                    FileSpec &file_spec) {
+  return Status("Unimplemented");
+}
+
+Status NativeProcessSunOS::GetFileLoadAddress(const llvm::StringRef &file_name,
+                                               lldb::addr_t &load_addr) {
+  load_addr = LLDB_INVALID_ADDRESS;
+  return Status();
+}
+
+void NativeProcessSunOS::SigchldHandler() {
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_PROCESS));
+  // Process all pending waitpid notifications.
+  int status;
+  ::pid_t wait_pid =
+      llvm::sys::RetryAfterSignal(-1, waitpid, GetID(), &status, WNOHANG);
+
+  if (wait_pid == 0)
+    return; // We are done.
+
+  if (wait_pid == -1) {
+    Status error(errno, eErrorTypePOSIX);
+    LLDB_LOG(log, "waitpid ({0}, &status, _) failed: {1}", GetID(), error);
+  }
+
+  WaitStatus wait_status = WaitStatus::Decode(status);
+  bool exited = wait_status.type == WaitStatus::Exit ||
+                (wait_status.type == WaitStatus::Signal &&
+                 wait_pid == static_cast<::pid_t>(GetID()));
+
+  LLDB_LOG(log,
+           "waitpid ({0}, &status, _) => pid = {1}, status = {2}, exited = {3}",
+           GetID(), wait_pid, status, exited);
+
+  if (exited)
+    MonitorExited(wait_pid, wait_status);
+  else {
+    assert(wait_status.type == WaitStatus::Stop);
+    MonitorCallback(wait_pid, wait_status.status);
+  }
+}
+
+bool NativeProcessSunOS::HasThreadNoLock(lldb::tid_t thread_id) {
+  for (const auto &thread : m_threads) {
+    assert(thread && "thread list should not contain NULL threads");
+    if (thread->GetID() == thread_id) {
+      // We have this thread.
+      return true;
+    }
+  }
+
+  // We don't have this thread.
+  return false;
+}
+
+NativeThreadSunOS &NativeProcessSunOS::AddThread(lldb::tid_t thread_id) {
+
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_THREAD));
+  LLDB_LOG(log, "pid {0} adding thread with tid {1}", GetID(), thread_id);
+
+  assert(!HasThreadNoLock(thread_id) &&
+         "attempted to add a thread by id that already exists");
+
+  // If this is the first thread, save it as the current thread
+  if (m_threads.empty())
+    SetCurrentThreadID(thread_id);
+
+  m_threads.push_back(llvm::make_unique<NativeThreadSunOS>(*this, thread_id));
+  return static_cast<NativeThreadSunOS &>(*m_threads.back());
+}
+
+Status NativeProcessSunOS::ReadMemory(lldb::addr_t addr, void *buf,
+                                       size_t size, size_t &bytes_read) {
+  assert(m_pr != NULL);
+
+  unsigned char *dst = static_cast<unsigned char *>(buf);
+  int len = 0;
+
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_MEMORY));
+  LLDB_LOG(log, "addr = {0}, buf = {1}, size = {2}", addr, buf, size);
+
+  Status error;
+  bytes_read = 0;
+
+  do {
+    len = Pread(m_pr, dst, size, addr);
+
+    switch (len) {
+    case -1:
+      error.SetErrorToErrno();
+      break;
+    case 0:
+      error.SetErrorString("short read");
+      break;
+    default:
+      bytes_read += len;
+      dst += len;
+      addr += len;
+      size -= len;
+    }
+  } while (len > 0 && size > 0);
+
+  return error;;
+}
+
+Status NativeProcessSunOS::WriteMemory(lldb::addr_t addr, const void *buf,
+                                        size_t size, size_t &bytes_written) {
+  assert(m_pr != NULL);
+
+  const unsigned char *src = static_cast<const unsigned char *>(buf);
+  int len = 0;
+
+  Log *log(ProcessPOSIXLog::GetLogIfAllCategoriesSet(POSIX_LOG_MEMORY));
+  LLDB_LOG(log, "addr = {0}, buf = {1}, size = {2}", addr, buf, size);
+
+  Status error;
+  bytes_written = 0;
+
+  do {
+    len = Pwrite(m_pr, src, size, addr);
+
+    switch (len) {
+    case -1:
+      error.SetErrorToErrno();
+      break;
+    case 0:
+      error.SetErrorString("short write");
+      break;
+    default:
+      bytes_written += len;
+      src += len;
+      addr += len;
+      size -= len;
+    }
+  } while (len > 0 && size > 0);
+
+  return error;
+}
+
+llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
+NativeProcessSunOS::GetAuxvData() const {
+  assert(m_pr != NULL);
+
+  const auxv_t *auxv = Pgetauxvec(m_pr);
+  const auxv_t *auxp = auxv;
+  size_t auxv_size = sizeof (auxv_t);
+
+  while (auxp->a_type != 0) {
+    auxv_size += sizeof (auxv_t);
+    auxp++;
+  }
+
+  // only need half the size for a i386 auxv
+  if (m_arch.GetMachine() == llvm::Triple::x86)
+    auxv_size /= 2;
+
+  ErrorOr<std::unique_ptr<WritableMemoryBuffer>> buf =
+      llvm::WritableMemoryBuffer::getNewMemBuffer(auxv_size);
+
+  if (m_arch.GetMachine() == llvm::Triple::x86) {
+    // This is beyond silly. The system always gives us the aux vector in our
+    // own native format, regardless of the native size of the debugged process.
+    // But AuxVector() insists on converting it itself and there's no non-
+    // intrusive way to fix that.
+    uint32_t *ptr = (uint32_t *)buf.get()->getBufferStart();
+
+    for (auxp = auxv; auxp->a_type != 0; auxp++) {
+      *ptr++ = (uint32_t)auxp->a_type;
+      *ptr++ = (uint32_t)auxp->a_un.a_val;
+    }
+
+    *ptr++ = 0;
+    *ptr++ = 0;
+
+  } else {
+    memcpy(buf.get()->getBufferStart(), auxv, auxv_size);
+  }
+
+  return std::move(buf);
+}
+
+int NativeProcessSunOS::LwpIterCB(void *arg, const lwpstatus_t *lwpstatus) {
+  NativeProcessSunOS *P = (NativeProcessSunOS *)arg;
+
+  P->AddThread(lwpstatus->pr_lwpid);
+
+  return 0;
+}
+
+Status NativeProcessSunOS::ReinitializeThreads() {
+  assert(m_pr != NULL);
+
+  // Clear old threads
+  m_threads.clear();
+
+  // Iterate over active threads only
+  if (Plwp_iter(m_pr, LwpIterCB, this) != 0) {
+    Status error;
+    error.SetErrorToErrno();
+    return error;
+  }
+
+  return Status();
+}
