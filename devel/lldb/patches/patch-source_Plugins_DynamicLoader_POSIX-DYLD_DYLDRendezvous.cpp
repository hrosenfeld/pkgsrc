$NetBSD$

--- source/Plugins/DynamicLoader/POSIX-DYLD/DYLDRendezvous.cpp.orig	2019-05-16 08:37:32.000000000 +0000
+++ source/Plugins/DynamicLoader/POSIX-DYLD/DYLDRendezvous.cpp
@@ -382,6 +382,7 @@ bool DYLDRendezvous::SOEntryIsMainExecut
   switch (triple.getOS()) {
   case llvm::Triple::FreeBSD:
   case llvm::Triple::NetBSD:
+  case llvm::Triple::Solaris:
     return entry.file_spec == m_exe_file_spec;
   case llvm::Triple::Linux:
     if (triple.isAndroid())
