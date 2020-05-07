$NetBSD$

--- source/Core/DumpDataExtractor.cpp.orig	2019-02-13 06:25:41.000000000 +0000
+++ source/Core/DumpDataExtractor.cpp
@@ -129,7 +129,7 @@ static lldb::offset_t DumpAPInt(Stream *
 }
 
 lldb::offset_t lldb_private::DumpDataExtractor(
-    const DataExtractor &DE, Stream *s, offset_t start_offset,
+    const DataExtractor &DE, Stream *s, lldb::offset_t start_offset,
     lldb::Format item_format, size_t item_byte_size, size_t item_count,
     size_t num_per_line, uint64_t base_addr,
     uint32_t item_bit_size,   // If zero, this is not a bitfield value, if
@@ -145,7 +145,7 @@ lldb::offset_t lldb_private::DumpDataExt
       item_byte_size = s->GetAddressByteSize();
   }
 
-  offset_t offset = start_offset;
+  lldb::offset_t offset = start_offset;
 
   if (item_format == eFormatInstruction) {
     TargetSP target_sp;
@@ -582,7 +582,7 @@ lldb::offset_t lldb_private::DumpDataExt
               const auto &semantics =
                   ast->getFloatTypeSemantics(ast->LongDoubleTy);
 
-              offset_t byte_size = item_byte_size;
+              lldb::offset_t byte_size = item_byte_size;
               if (&semantics == &llvm::APFloatBase::x87DoubleExtended())
                 byte_size = (llvm::APFloat::getSizeInBits(semantics) + 7) / 8;
 
