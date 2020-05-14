$NetBSD$

--- source/Plugins/Process/gdb-remote/GDBRemoteCommunicationClient.cpp.orig	2019-06-30 19:00:09.000000000 +0000
+++ source/Plugins/Process/gdb-remote/GDBRemoteCommunicationClient.cpp
@@ -1893,7 +1893,7 @@ bool GDBRemoteCommunicationClient::Decod
       } else if (name.equals("euid")) {
         uint32_t uid = UINT32_MAX;
         value.getAsInteger(0, uid);
-        process_info.SetEffectiveGroupID(uid);
+        process_info.SetEffectiveUserID(uid);
       } else if (name.equals("gid")) {
         uint32_t gid = UINT32_MAX;
         value.getAsInteger(0, gid);
