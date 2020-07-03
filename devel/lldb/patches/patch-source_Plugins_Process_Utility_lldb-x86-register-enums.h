$NetBSD$

--- source/Plugins/Process/Utility/lldb-x86-register-enums.h.orig	2019-04-10 20:48:55.000000000 +0000
+++ source/Plugins/Process/Utility/lldb-x86-register-enums.h
@@ -106,7 +106,7 @@ enum {
   lldb_bnd1_i386,
   lldb_bnd2_i386,
   lldb_bnd3_i386,
-  k_last_mpxr = lldb_bnd3_i386,
+  k_last_mpxr_i386 = lldb_bnd3_i386,
 
   k_first_mpxc_i386,
   lldb_bndcfgu_i386 = k_first_mpxc_i386,
