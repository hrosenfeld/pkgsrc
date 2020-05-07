$NetBSD$

--- source/Plugins/ABI/SysV-mips64/ABISysV_mips64.cpp.orig	2019-05-24 00:44:33.000000000 +0000
+++ source/Plugins/ABI/SysV-mips64/ABISysV_mips64.cpp
@@ -972,7 +972,7 @@ ValueObjectSP ABISysV_mips64::GetReturnV
 
             DataExtractor *copy_from_extractor = nullptr;
             uint64_t return_value[2];
-            offset_t offset = 0;
+            lldb::offset_t offset = 0;
 
             if (idx == 0) {
               // This case is for long double type.
