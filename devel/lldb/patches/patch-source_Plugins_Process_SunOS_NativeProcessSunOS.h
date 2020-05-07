$NetBSD$

--- source/Plugins/Process/SunOS/NativeProcessSunOS.h.orig	2020-05-15 14:03:42.243897700 +0000
+++ source/Plugins/Process/SunOS/NativeProcessSunOS.h
@@ -0,0 +1,165 @@
+//===-- NativeProcessSunOS.h --------------------------------- -*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef liblldb_NativeProcessSunOS_H_
+#define liblldb_NativeProcessSunOS_H_
+
+#include "lldb/Host/Pipe.h"
+#include "lldb/Target/MemoryRegionInfo.h"
+#include "lldb/Utility/ArchSpec.h"
+#include "lldb/Utility/FileSpec.h"
+
+#include "NativeThreadSunOS.h"
+#include "lldb/Host/common/NativeProcessProtocol.h"
+
+#include <libproc.h>
+
+namespace lldb_private {
+namespace process_sunos {
+/// \class NativeProcessSunOS
+/// Manages communication with the inferior (debugee) process.
+///
+/// Upon construction, this class prepares and launches an inferior process
+/// for debugging.
+///
+/// Changes in the inferior process state are broadcasted.
+class NativeProcessSunOS : public NativeProcessProtocol {
+public:
+  class Factory : public NativeProcessProtocol::Factory {
+  public:
+    llvm::Expected<std::unique_ptr<NativeProcessProtocol>>
+    Launch(ProcessLaunchInfo &launch_info, NativeDelegate &native_delegate,
+           MainLoop &mainloop) const override;
+
+    llvm::Expected<std::unique_ptr<NativeProcessProtocol>>
+    Attach(lldb::pid_t pid, NativeDelegate &native_delegate,
+           MainLoop &mainloop) const override;
+  };
+
+  // NativeProcessProtocol Interface
+  Status Resume(const ResumeActionList &resume_actions) override;
+
+  Status Halt() override;
+
+  Status Detach() override;
+
+  Status Signal(int signo) override;
+
+  Status Interrupt() override;
+
+  Status Kill() override;
+
+  Status GetMemoryRegionInfo(lldb::addr_t load_addr,
+                             MemoryRegionInfo &range_info) override;
+
+  Status ReadMemory(lldb::addr_t addr, void *buf, size_t size,
+                    size_t &bytes_read) override;
+
+  Status WriteMemory(lldb::addr_t addr, const void *buf, size_t size,
+                     size_t &bytes_written) override;
+
+  Status AllocateMemory(size_t size, uint32_t permissions,
+                        lldb::addr_t &addr) override;
+
+  Status DeallocateMemory(lldb::addr_t addr) override;
+
+  lldb::addr_t GetSharedLibraryInfoAddress() override;
+
+  size_t UpdateThreads() override;
+
+  const ArchSpec &GetArchitecture() const override { return m_arch; }
+
+  Status SetBreakpoint(lldb::addr_t addr, uint32_t size,
+                       bool hardware) override;
+
+  Status SetWatchpoint(lldb::addr_t addr, size_t size,
+                       uint32_t watch_flags, bool hardware);
+
+  Status RemoveWatchpoint(lldb::addr_t addr);
+
+  Status GetLoadedModuleFileSpec(const char *module_path,
+                                 FileSpec &file_spec) override;
+
+  Status GetFileLoadAddress(const llvm::StringRef &file_name,
+                            lldb::addr_t &load_addr) override;
+
+  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
+  GetAuxvData() const override;
+
+  struct ps_prochandle *GetProcHdl() const { return m_pr; };
+
+private:
+  struct SoftwareWatchpoint {
+    uint32_t ref_count;
+    prwatch_t prw;
+  };
+
+  std::unordered_map<lldb::addr_t, SoftwareWatchpoint> m_software_watchpoints;
+  prwatch_t *m_last_watchpoint;
+
+  Pipe m_pwait_pipe;
+  pthread_t m_pwait_thread;
+  MainLoop::ReadHandleUP m_pwait_reader_handle;
+  MainLoop::SignalHandleUP m_sigchld_handle;
+  MainLoop &m_mainloop;
+  ArchSpec m_arch;
+  LazyBool m_supports_mem_region = eLazyBoolCalculate;
+  std::vector<std::pair<MemoryRegionInfo, FileSpec>> m_mem_region_cache;
+  struct ps_prochandle *m_pr;
+
+  // Private Instance Methods
+  NativeProcessSunOS(::pid_t pid, int terminal_fd, NativeDelegate &delegate,
+                     const ArchSpec &arch, MainLoop &mainloop,
+                     struct ps_prochandle *);
+
+  bool HasThreadNoLock(lldb::tid_t thread_id);
+
+  NativeThreadSunOS &AddThread(lldb::tid_t thread_id);
+
+  void MonitorCallback(lldb::pid_t pid, int signal);
+  void MonitorExited(lldb::pid_t pid, WaitStatus status);
+  void MonitorSIGSTOP(lldb::pid_t pid);
+  void MonitorSIGTRAP(lldb::pid_t pid);
+  void MonitorSignal(lldb::pid_t pid, int signal);
+
+  Status PopulateMemoryRegionCache();
+  int PopulateOneMemoryRegion(const prmap_t *prmap, const char *name);
+  void SigchldHandler();
+
+  Status HandlePwaitResult();
+  Status StartPwaitThread();
+  static void *PwaitThread(void *arg);
+  void *PwaitThread();
+
+  Status ReinitializeThreads();
+
+  static int MapIterCB(void *arg, const prmap_t *prmap, const char *name);
+  static int LwpIterCB(void *arg, const lwpstatus_t *lwps);
+
+  // interface for software breakpoints and watchpoints
+  size_t GetSoftwareBreakpointPCOffset() { return 0; };
+  Status SetSoftwareBreakpoint(lldb::addr_t addr, uint32_t size_hint);
+  Status RemoveSoftwareBreakpoint(lldb::addr_t addr);
+
+  llvm::Expected<llvm::ArrayRef<uint8_t>>
+  GetSoftwareBreakpointTrapOpcode(size_t size_hint);
+
+  llvm::Expected<SoftwareBreakpoint>
+  EnableSoftwareBreakpoint(lldb::addr_t addr, uint32_t size_hint);
+
+  prwatch_t *GetSoftwareWatchpoint(lldb::addr_t addr);
+  Status SetSoftwareWatchpoint(lldb::addr_t addr, size_t size, uint32_t flags);
+  Status RemoveSoftwareWatchpoint(lldb::addr_t addr);
+  llvm::Expected<SoftwareWatchpoint>
+  EnableSoftwareWatchpoint(lldb::addr_t addr, size_t size, uint32_t flags);
+};
+
+} // namespace process_sunos
+} // namespace lldb_private
+
+#endif // #ifndef liblldb_NativeProcessSunOS_H_
