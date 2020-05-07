$NetBSD$

--- source/Host/posix/PipePosix.cpp.orig	2019-03-21 19:35:55.000000000 +0000
+++ source/Host/posix/PipePosix.cpp
@@ -39,7 +39,7 @@ enum PIPES { READ, WRITE }; // Constants
 // pipe2 is supported by a limited set of platforms
 // TODO: Add more platforms that support pipe2.
 #if defined(__linux__) || (defined(__FreeBSD__) && __FreeBSD__ >= 10) ||       \
-    defined(__NetBSD__)
+    defined(__NetBSD__) || defined(__sun)
 #define PIPE2_SUPPORTED 1
 #else
 #define PIPE2_SUPPORTED 0
