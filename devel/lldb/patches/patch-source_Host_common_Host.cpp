$NetBSD$

--- source/Host/common/Host.cpp.orig	2019-07-08 07:07:05.000000000 +0000
+++ source/Host/common/Host.cpp
@@ -28,7 +28,7 @@
 
 #if defined(__linux__) || defined(__FreeBSD__) ||                              \
     defined(__FreeBSD_kernel__) || defined(__APPLE__) ||                       \
-    defined(__NetBSD__) || defined(__OpenBSD__)
+    defined(__NetBSD__) || defined(__OpenBSD__) || defined(__sun)
 #if !defined(__ANDROID__)
 #include <spawn.h>
 #endif
@@ -178,7 +178,8 @@ static thread_result_t MonitorChildProce
   delete info;
 
   int status = -1;
-#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__OpenBSD__)
+#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__OpenBSD__) \
+	|| defined(__sun)
 #define __WALL 0
 #endif
   const int options = __WALL;
@@ -414,7 +415,7 @@ FileSpec Host::GetModuleFileSpecForHostA
   FileSpec module_filespec;
 #if !defined(__ANDROID__)
   Dl_info info;
-  if (::dladdr(host_addr, &info)) {
+  if (::dladdr((void *)host_addr, &info)) {
     if (info.dli_fname) {
       module_filespec.SetFile(info.dli_fname, FileSpec::Style::native);
       FileSystem::Instance().Resolve(module_filespec);
