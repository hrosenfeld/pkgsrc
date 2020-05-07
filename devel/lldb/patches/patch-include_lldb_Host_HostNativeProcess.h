$NetBSD$

--- include/lldb/Host/HostNativeProcess.h.orig	2019-01-19 08:50:56.000000000 +0000
+++ include/lldb/Host/HostNativeProcess.h
@@ -14,6 +14,11 @@
 namespace lldb_private {
 typedef HostProcessWindows HostNativeProcess;
 }
+#elif defined(__sun)
+#include "lldb/Host/sunos/HostProcessSunOS.h"
+namespace lldb_private {
+typedef HostProcessSunOS HostNativeProcess;
+}
 #else
 #include "lldb/Host/posix/HostProcessPosix.h"
 namespace lldb_private {
