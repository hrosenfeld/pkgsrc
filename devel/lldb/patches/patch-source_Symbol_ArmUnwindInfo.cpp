$NetBSD$

--- source/Symbol/ArmUnwindInfo.cpp.orig	2019-04-16 08:06:56.000000000 +0000
+++ source/Symbol/ArmUnwindInfo.cpp
@@ -54,7 +54,7 @@ ArmUnwindInfo::ArmUnwindInfo(ObjectFile
 
   addr_t exidx_base_addr = m_arm_exidx_sp->GetFileAddress();
 
-  offset_t offset = 0;
+  lldb::offset_t offset = 0;
   while (m_arm_exidx_data.ValidOffset(offset)) {
     lldb::addr_t file_addr = exidx_base_addr + offset;
     lldb::addr_t addr = exidx_base_addr + (addr_t)offset +
