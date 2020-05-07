$NetBSD$

--- source/Initialization/SystemInitializerCommon.cpp.orig	2019-06-13 04:35:22.000000000 +0000
+++ source/Initialization/SystemInitializerCommon.cpp
@@ -18,7 +18,7 @@
 #include "lldb/Utility/Timer.h"
 #include "lldb/lldb-private.h"
 
-#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
+#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__sun)
 #include "Plugins/Process/POSIX/ProcessPOSIXLog.h"
 #endif
 
@@ -98,7 +98,7 @@ llvm::Error SystemInitializerCommon::Ini
 
   process_gdb_remote::ProcessGDBRemoteLog::Initialize();
 
-#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
+#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__sun)
   ProcessPOSIXLog::Initialize();
 #endif
 #if defined(_WIN32)
