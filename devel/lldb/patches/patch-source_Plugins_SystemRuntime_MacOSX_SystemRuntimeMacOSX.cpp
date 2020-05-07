$NetBSD$

--- source/Plugins/SystemRuntime/MacOSX/SystemRuntimeMacOSX.cpp.orig	2019-06-19 21:33:44.000000000 +0000
+++ source/Plugins/SystemRuntime/MacOSX/SystemRuntimeMacOSX.cpp
@@ -783,7 +783,7 @@ SystemRuntimeMacOSX::GetPendingItemRefsF
           //   introspection_dispatch_pending_item_info_s items[];
           //   }
 
-          offset_t offset = 0;
+          lldb::offset_t offset = 0;
           int i = 0;
           uint32_t version = extractor.GetU32(&offset);
           if (version == 1) {
@@ -890,7 +890,7 @@ void SystemRuntimeMacOSX::PopulateQueues
     DataExtractor extractor(data.GetBytes(), data.GetByteSize(),
                             m_process->GetByteOrder(),
                             m_process->GetAddressByteSize());
-    offset_t offset = 0;
+    lldb::offset_t offset = 0;
     uint64_t queues_read = 0;
 
     // The information about the queues is stored in this format (v1): typedef
@@ -907,7 +907,7 @@ void SystemRuntimeMacOSX::PopulateQueues
     // } introspection_dispatch_queue_info_s;
 
     while (queues_read < count && offset < queues_buffer_size) {
-      offset_t start_of_this_item = offset;
+      lldb::offset_t start_of_this_item = offset;
 
       uint32_t offset_to_next = extractor.GetU32(&offset);
 
@@ -924,7 +924,7 @@ void SystemRuntimeMacOSX::PopulateQueues
       if (queue_label == nullptr)
         queue_label = "";
 
-      offset_t start_of_next_item = start_of_this_item + offset_to_next;
+      lldb::offset_t start_of_next_item = start_of_this_item + offset_to_next;
       offset = start_of_next_item;
 
       if (log)
@@ -951,7 +951,7 @@ SystemRuntimeMacOSX::ItemInfo SystemRunt
     lldb_private::DataExtractor &extractor) {
   ItemInfo item;
 
-  offset_t offset = 0;
+  lldb::offset_t offset = 0;
 
   item.item_that_enqueued_this = extractor.GetPointer(&offset);
   item.function_or_block = extractor.GetPointer(&offset);
