$NetBSD$

--- source/Plugins/Process/gdb-remote/GDBRemoteCommunicationServerCommon.cpp.orig	2019-07-01 12:41:20.000000000 +0000
+++ source/Plugins/Process/gdb-remote/GDBRemoteCommunicationServerCommon.cpp
@@ -822,7 +822,7 @@ GDBRemoteCommunicationServerCommon::Hand
   response.PutCString(";QThreadSuffixSupported+");
   response.PutCString(";QListThreadsInStopReply+");
   response.PutCString(";qEcho+");
-#if defined(__linux__) || defined(__NetBSD__)
+#if defined(__linux__) || defined(__NetBSD__) || defined(__sun)
   response.PutCString(";QPassSignals+");
   response.PutCString(";qXfer:auxv:read+");
 #endif
