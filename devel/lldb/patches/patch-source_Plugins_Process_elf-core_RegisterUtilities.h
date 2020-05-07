$NetBSD$

--- source/Plugins/Process/elf-core/RegisterUtilities.h.orig	2019-04-01 15:08:24.000000000 +0000
+++ source/Plugins/Process/elf-core/RegisterUtilities.h
@@ -92,6 +92,25 @@ enum {
 };
 }
 
+namespace SUNOS {
+enum {
+  NT_PRSTATUS = 1,
+  NT_PRFPREG = 2,
+  NT_PRPSINFO = 3,
+  NT_AUXV = 6,
+  NT_LWPSTATUS = 16,
+  NT_LWPSINFO = 17,
+  NT_LWPNAME = 25,
+
+  GPR_OFF_x86 = 0x158,
+  GPR_LEN_x86 = 0x4c,
+  FPR_LEN_x86 = 0x17c,
+  GPR_OFF_x86_64 = 0x220,
+  GPR_LEN_x86_64 = 0xe0,
+  FPR_LEN_x86_64 = 0x200,
+};     
+}
+
 struct CoreNote {
   ELFNote info;
   DataExtractor data;
