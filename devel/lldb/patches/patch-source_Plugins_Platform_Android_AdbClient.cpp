$NetBSD$

--- source/Plugins/Platform/Android/AdbClient.cpp.orig	2019-01-19 08:50:56.000000000 +0000
+++ source/Plugins/Platform/Android/AdbClient.cpp
@@ -527,7 +527,7 @@ Status AdbClient::SyncService::internalS
 
   DataExtractor extractor(&buffer[0], buffer.size(), eByteOrderLittle,
                           sizeof(void *));
-  offset_t offset = 0;
+  lldb::offset_t offset = 0;
 
   const void *command = extractor.GetData(&offset, stat_len);
   if (!command)
@@ -611,7 +611,7 @@ Status AdbClient::SyncService::ReadSyncH
   if (error.Success()) {
     response_id.assign(&buffer[0], 4);
     DataExtractor extractor(&buffer[4], 4, eByteOrderLittle, sizeof(void *));
-    offset_t offset = 0;
+    lldb::offset_t offset = 0;
     data_len = extractor.GetU32(&offset);
   }
 
