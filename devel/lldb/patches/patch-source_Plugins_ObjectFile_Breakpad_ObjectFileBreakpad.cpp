$NetBSD$

--- source/Plugins/ObjectFile/Breakpad/ObjectFileBreakpad.cpp.orig	2019-04-09 08:05:11.000000000 +0000
+++ source/Plugins/ObjectFile/Breakpad/ObjectFileBreakpad.cpp
@@ -58,8 +58,9 @@ ConstString ObjectFileBreakpad::GetPlugi
 }
 
 ObjectFile *ObjectFileBreakpad::CreateInstance(
-    const ModuleSP &module_sp, DataBufferSP &data_sp, offset_t data_offset,
-    const FileSpec *file, offset_t file_offset, offset_t length) {
+    const ModuleSP &module_sp, DataBufferSP &data_sp,
+    lldb::offset_t data_offset, const FileSpec *file,
+    lldb::offset_t file_offset, lldb::offset_t length) {
   if (!data_sp) {
     data_sp = MapFileData(*file, length, file_offset);
     if (!data_sp)
@@ -91,8 +92,8 @@ ObjectFile *ObjectFileBreakpad::CreateMe
 }
 
 size_t ObjectFileBreakpad::GetModuleSpecifications(
-    const FileSpec &file, DataBufferSP &data_sp, offset_t data_offset,
-    offset_t file_offset, offset_t length, ModuleSpecList &specs) {
+    const FileSpec &file, DataBufferSP &data_sp, lldb::offset_t data_offset,
+    lldb::offset_t file_offset, lldb::offset_t length, ModuleSpecList &specs) {
   auto text = toStringRef(data_sp->GetData());
   llvm::Optional<Header> header = Header::parse(text);
   if (!header)
@@ -105,9 +106,10 @@ size_t ObjectFileBreakpad::GetModuleSpec
 
 ObjectFileBreakpad::ObjectFileBreakpad(const ModuleSP &module_sp,
                                        DataBufferSP &data_sp,
-                                       offset_t data_offset,
-                                       const FileSpec *file, offset_t offset,
-                                       offset_t length, ArchSpec arch,
+                                       lldb::offset_t data_offset,
+                                       const FileSpec *file,
+                                       lldb::offset_t offset,
+                                       lldb::offset_t length, ArchSpec arch,
                                        UUID uuid)
     : ObjectFile(module_sp, file, offset, length, data_sp, data_offset),
       m_arch(std::move(arch)), m_uuid(std::move(uuid)) {}
@@ -128,14 +130,14 @@ void ObjectFileBreakpad::CreateSections(
   m_sections_up = llvm::make_unique<SectionList>();
 
   llvm::Optional<Record::Kind> current_section;
-  offset_t section_start;
+  lldb::offset_t section_start;
   llvm::StringRef text = toStringRef(m_data.GetData());
   uint32_t next_section_id = 1;
   auto maybe_add_section = [&](const uint8_t *end_ptr) {
     if (!current_section)
       return; // We have been called before parsing the first line.
 
-    offset_t end_offset = end_ptr - m_data.GetDataStart();
+    lldb::offset_t end_offset = end_ptr - m_data.GetDataStart();
     auto section_sp = std::make_shared<Section>(
         GetModule(), this, next_section_id++,
         ConstString(toString(*current_section)), eSectionTypeOther,
