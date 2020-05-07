$NetBSD$

--- source/Plugins/ABI/SysV-ppc64/ABISysV_ppc64.cpp.orig	2020-02-27 19:50:52.814352052 +0000
+++ source/Plugins/ABI/SysV-ppc64/ABISysV_ppc64.cpp
@@ -647,7 +647,7 @@ private:
 
     DataExtractor de(&raw_data, sizeof(raw_data), m_byte_order, m_addr_size);
 
-    offset_t offset = 0;
+    lldb::offset_t offset = 0;
     llvm::Optional<uint64_t> byte_size = type.GetByteSize(nullptr);
     if (!byte_size)
       return {};
