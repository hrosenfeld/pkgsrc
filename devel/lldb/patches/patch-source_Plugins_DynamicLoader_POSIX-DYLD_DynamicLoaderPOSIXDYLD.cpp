$NetBSD$

--- source/Plugins/DynamicLoader/POSIX-DYLD/DynamicLoaderPOSIXDYLD.cpp.orig	2020-05-26 13:48:15.523492630 +0000
+++ source/Plugins/DynamicLoader/POSIX-DYLD/DynamicLoaderPOSIXDYLD.cpp
@@ -60,6 +60,7 @@ DynamicLoader *DynamicLoaderPOSIXDYLD::C
         process->GetTarget().GetArchitecture().GetTriple();
     if (triple_ref.getOS() == llvm::Triple::FreeBSD ||
         triple_ref.getOS() == llvm::Triple::Linux ||
+        triple_ref.getOS() == llvm::Triple::Solaris ||
         triple_ref.getOS() == llvm::Triple::NetBSD)
       create = true;
   }
