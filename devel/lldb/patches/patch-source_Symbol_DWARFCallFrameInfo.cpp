$NetBSD$

--- source/Symbol/DWARFCallFrameInfo.cpp.orig	2019-05-22 00:06:44.000000000 +0000
+++ source/Symbol/DWARFCallFrameInfo.cpp
@@ -28,7 +28,7 @@ using namespace lldb_private;
 // Used for calls when the value type is specified by a DWARF EH Frame pointer
 // encoding.
 static uint64_t
-GetGNUEHPointer(const DataExtractor &DE, offset_t *offset_ptr,
+GetGNUEHPointer(const DataExtractor &DE, lldb::offset_t *offset_ptr,
                 uint32_t eh_ptr_enc, addr_t pc_rel_addr, addr_t text_addr,
                 addr_t data_addr) //, BSDRelocs *data_relocs) const
 {
@@ -588,7 +588,7 @@ bool DWARFCallFrameInfo::FDEToUnwindPlan
   if (cie->augmentation[0] == 'z') {
     uint32_t aug_data_len = (uint32_t)m_cfi_data.GetULEB128(&offset);
     if (aug_data_len != 0 && cie->lsda_addr_encoding != DW_EH_PE_omit) {
-      offset_t saved_offset = offset;
+      lldb::offset_t saved_offset = offset;
       lsda_data_file_address =
           GetGNUEHPointer(m_cfi_data, &offset, cie->lsda_addr_encoding,
                           pc_rel_addr, text_addr, data_addr);
