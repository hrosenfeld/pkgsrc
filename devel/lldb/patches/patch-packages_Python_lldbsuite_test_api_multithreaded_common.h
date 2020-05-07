$NetBSD$

--- packages/Python/lldbsuite/test/api/multithreaded/common.h.orig	2015-12-07 21:21:12.000000000 +0000
+++ packages/Python/lldbsuite/test/api/multithreaded/common.h
@@ -58,7 +58,7 @@ public:
 
 /// Allocates a char buffer with the current working directory
 inline char* get_working_dir() {
-#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
+#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__sun)
     return getwd(0);
 #else
     return get_current_dir_name();
