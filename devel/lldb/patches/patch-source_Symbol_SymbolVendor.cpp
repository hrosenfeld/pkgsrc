$NetBSD$

--- source/Symbol/SymbolVendor.cpp.orig	2019-04-10 20:48:55.000000000 +0000
+++ source/Symbol/SymbolVendor.cpp
@@ -44,7 +44,7 @@ SymbolVendor *SymbolVendor::FindPlugin(c
   FileSpec sym_spec = module_sp->GetSymbolFileFileSpec();
   if (sym_spec && sym_spec != module_sp->GetObjectFile()->GetFileSpec()) {
     DataBufferSP data_sp;
-    offset_t data_offset = 0;
+    lldb::offset_t data_offset = 0;
     sym_objfile_sp = ObjectFile::FindPlugin(
         module_sp, &sym_spec, 0, FileSystem::Instance().GetByteSize(sym_spec),
         data_sp, data_offset);
