$NetBSD$

--- include/lldb/Host/HostProcess.h.orig	2020-04-20 17:59:45.231829613 +0000
+++ include/lldb/Host/HostProcess.h
@@ -35,6 +35,9 @@ class HostProcess {
 public:
   HostProcess();
   HostProcess(lldb::process_t process);
+#ifdef __sun
+  HostProcess(::pid_t pid);
+#endif
   ~HostProcess();
 
   Status Terminate();
