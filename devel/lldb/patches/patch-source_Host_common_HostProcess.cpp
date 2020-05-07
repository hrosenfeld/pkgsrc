$NetBSD$

--- source/Host/common/HostProcess.cpp.orig	2020-04-20 18:18:55.118973275 +0000
+++ source/Host/common/HostProcess.cpp
@@ -18,6 +18,11 @@ HostProcess::HostProcess() : m_native_pr
 HostProcess::HostProcess(lldb::process_t process)
     : m_native_process(new HostNativeProcess(process)) {}
 
+#ifdef __sun
+HostProcess::HostProcess(::pid_t pid)
+    : m_native_process(new HostNativeProcess(pid)) {}
+#endif
+
 HostProcess::~HostProcess() {}
 
 Status HostProcess::Terminate() { return m_native_process->Terminate(); }
