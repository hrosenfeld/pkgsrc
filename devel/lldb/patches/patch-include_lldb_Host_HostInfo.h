$NetBSD$

--- include/lldb/Host/HostInfo.h.orig	2019-05-13 04:42:32.000000000 +0000
+++ include/lldb/Host/HostInfo.h
@@ -55,6 +55,9 @@
 #elif defined(__APPLE__)
 #include "lldb/Host/macosx/HostInfoMacOSX.h"
 #define HOST_INFO_TYPE HostInfoMacOSX
+#elif defined(__sun)
+#include "lldb/Host/sunos/HostInfoSunOS.h"
+#define HOST_INFO_TYPE HostInfoSunOS
 #else
 #include "lldb/Host/posix/HostInfoPosix.h"
 #define HOST_INFO_TYPE HostInfoPosix
