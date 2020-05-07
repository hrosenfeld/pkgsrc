$NetBSD$

--- tools/lldb-server/lldb-gdbserver.cpp.orig	2019-05-23 11:14:47.000000000 +0000
+++ tools/lldb-server/lldb-gdbserver.cpp
@@ -39,6 +39,8 @@
 #include "Plugins/Process/Linux/NativeProcessLinux.h"
 #elif defined(__NetBSD__)
 #include "Plugins/Process/NetBSD/NativeProcessNetBSD.h"
+#elif defined(__sun)
+#include "Plugins/Process/SunOS/NativeProcessSunOS.h"
 #endif
 
 #ifndef LLGS_PROGRAM_NAME
@@ -60,6 +62,8 @@ namespace {
 typedef process_linux::NativeProcessLinux::Factory NativeProcessFactory;
 #elif defined(__NetBSD__)
 typedef process_netbsd::NativeProcessNetBSD::Factory NativeProcessFactory;
+#elif defined(__sun)
+typedef process_sunos::NativeProcessSunOS::Factory NativeProcessFactory;
 #else
 // Dummy implementation to make sure the code compiles
 class NativeProcessFactory : public NativeProcessProtocol::Factory {
@@ -383,7 +387,7 @@ int main_gdbserver(int argc, char *argv[
 
   bool show_usage = false;
   int option_error = 0;
-#if __GLIBC__
+#if defined(__GLIBC__) || defined(__sun)
   optind = 0;
 #else
   optreset = 1;
