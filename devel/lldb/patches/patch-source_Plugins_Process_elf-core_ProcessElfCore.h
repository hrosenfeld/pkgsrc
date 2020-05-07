$NetBSD$

--- source/Plugins/Process/elf-core/ProcessElfCore.h.orig	2019-06-11 20:16:13.000000000 +0000
+++ source/Plugins/Process/elf-core/ProcessElfCore.h
@@ -163,6 +163,7 @@ private:
   llvm::Error parseNetBSDNotes(llvm::ArrayRef<lldb_private::CoreNote> notes);
   llvm::Error parseOpenBSDNotes(llvm::ArrayRef<lldb_private::CoreNote> notes);
   llvm::Error parseLinuxNotes(llvm::ArrayRef<lldb_private::CoreNote> notes);
+  llvm::Error parseSunOSNotes(llvm::ArrayRef<lldb_private::CoreNote> notes);
 };
 
 #endif // liblldb_ProcessElfCore_h_
