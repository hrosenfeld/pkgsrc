$NetBSD$

--- source/Symbol/CompactUnwindInfo.cpp.orig	2019-05-23 11:14:47.000000000 +0000
+++ source/Symbol/CompactUnwindInfo.cpp
@@ -285,7 +285,7 @@ void CompactUnwindInfo::ScanIndex(const
   }
 
   if (m_unwindinfo_data.GetByteSize() > 0) {
-    offset_t offset = 0;
+    lldb::offset_t offset = 0;
 
     // struct unwind_info_section_header
     // {
@@ -386,12 +386,12 @@ uint32_t CompactUnwindInfo::GetLSDAForFu
   //         uint32_t        lsdaOffset;
   // };
 
-  offset_t first_entry = lsda_offset;
+  lldb::offset_t first_entry = lsda_offset;
   uint32_t low = 0;
   uint32_t high = lsda_count;
   while (low < high) {
     uint32_t mid = (low + high) / 2;
-    offset_t offset = first_entry + (mid * 8);
+    lldb::offset_t offset = first_entry + (mid * 8);
     uint32_t mid_func_offset =
         m_unwindinfo_data.GetU32(&offset); // functionOffset
     uint32_t mid_lsda_offset = m_unwindinfo_data.GetU32(&offset); // lsdaOffset
@@ -415,14 +415,14 @@ lldb::offset_t CompactUnwindInfo::Binary
   //     uint32_t                    functionOffset;
   //     compact_unwind_encoding_t    encoding;
 
-  offset_t first_entry = entry_page_offset;
+  lldb::offset_t first_entry = entry_page_offset;
 
   uint32_t low = 0;
   uint32_t high = entry_count;
   uint32_t last = high - 1;
   while (low < high) {
     uint32_t mid = (low + high) / 2;
-    offset_t offset = first_entry + (mid * 8);
+    lldb::offset_t offset = first_entry + (mid * 8);
     uint32_t mid_func_offset =
         m_unwindinfo_data.GetU32(&offset); // functionOffset
     uint32_t next_func_offset = 0;
@@ -451,14 +451,14 @@ uint32_t CompactUnwindInfo::BinarySearch
     uint32_t entry_page_offset, uint32_t entry_count,
     uint32_t function_offset_to_find, uint32_t function_offset_base,
     uint32_t *entry_func_start_offset, uint32_t *entry_func_end_offset) {
-  offset_t first_entry = entry_page_offset;
+  lldb::offset_t first_entry = entry_page_offset;
 
   uint32_t low = 0;
   uint32_t high = entry_count;
   uint32_t last = high - 1;
   while (low < high) {
     uint32_t mid = (low + high) / 2;
-    offset_t offset = first_entry + (mid * 4);
+    lldb::offset_t offset = first_entry + (mid * 4);
     uint32_t entry = m_unwindinfo_data.GetU32(&offset); // entry
     uint32_t mid_func_offset = UNWIND_INFO_COMPRESSED_ENTRY_FUNC_OFFSET(entry);
     mid_func_offset += function_offset_base;
@@ -536,11 +536,11 @@ bool CompactUnwindInfo::GetCompactUnwind
     unwind_info.valid_range_offset_end = next_it->function_offset;
   }
 
-  offset_t second_page_offset = it->second_level;
-  offset_t lsda_array_start = it->lsda_array_start;
-  offset_t lsda_array_count = (it->lsda_array_end - it->lsda_array_start) / 8;
+  lldb::offset_t second_page_offset = it->second_level;
+  lldb::offset_t lsda_array_start = it->lsda_array_start;
+  lldb::offset_t lsda_array_count = (it->lsda_array_end - it->lsda_array_start) / 8;
 
-  offset_t offset = second_page_offset;
+  lldb::offset_t offset = second_page_offset;
   uint32_t kind = m_unwindinfo_data.GetU32(
       &offset); // UNWIND_SECOND_LEVEL_REGULAR or UNWIND_SECOND_LEVEL_COMPRESSED
 
@@ -559,7 +559,7 @@ bool CompactUnwindInfo::GetCompactUnwind
         m_unwindinfo_data.GetU16(&offset);                    // entryPageOffset
     uint16_t entry_count = m_unwindinfo_data.GetU16(&offset); // entryCount
 
-    offset_t entry_offset = BinarySearchRegularSecondPage(
+    lldb::offset_t entry_offset = BinarySearchRegularSecondPage(
         second_page_offset + entry_page_offset, entry_count, function_offset,
         &unwind_info.valid_range_offset_start,
         &unwind_info.valid_range_offset_end);
@@ -586,7 +586,7 @@ bool CompactUnwindInfo::GetCompactUnwind
       if (personality_index > 0) {
         personality_index--;
         if (personality_index < m_unwind_header.personality_array_count) {
-          offset_t offset = m_unwind_header.personality_array_offset;
+          lldb::offset_t offset = m_unwind_header.personality_array_offset;
           offset += 4 * personality_index;
           SectionList *sl = m_objfile.GetSectionList();
           if (sl) {
@@ -670,7 +670,7 @@ bool CompactUnwindInfo::GetCompactUnwind
       if (personality_index > 0) {
         personality_index--;
         if (personality_index < m_unwind_header.personality_array_count) {
-          offset_t offset = m_unwind_header.personality_array_offset;
+          lldb::offset_t offset = m_unwind_header.personality_array_offset;
           offset += 4 * personality_index;
           SectionList *sl = m_objfile.GetSectionList();
           if (sl) {
