$NetBSD$

--- source/Plugins/Process/elf-core/ThreadElfCore.cpp.orig	2019-04-10 20:48:55.000000000 +0000
+++ source/Plugins/Process/elf-core/ThreadElfCore.cpp
@@ -25,6 +25,8 @@
 #include "Plugins/Process/Utility/RegisterContextNetBSD_x86_64.h"
 #include "Plugins/Process/Utility/RegisterContextOpenBSD_i386.h"
 #include "Plugins/Process/Utility/RegisterContextOpenBSD_x86_64.h"
+#include "Plugins/Process/Utility/RegisterContextSunOS_i386.h"
+#include "Plugins/Process/Utility/RegisterContextSunOS_x86_64.h"
 #include "Plugins/Process/Utility/RegisterInfoPOSIX_arm.h"
 #include "Plugins/Process/Utility/RegisterInfoPOSIX_arm64.h"
 #include "Plugins/Process/Utility/RegisterInfoPOSIX_ppc64le.h"
@@ -176,6 +178,20 @@ ThreadElfCore::CreateRegisterContextForF
       break;
     }
 
+    case llvm::Triple::Solaris: {
+      switch (arch.GetMachine()) {
+      case llvm::Triple::x86:
+        reg_interface = new RegisterContextSunOS_i386(arch);
+        break;
+      case llvm::Triple::x86_64:
+        reg_interface = new RegisterContextSunOS_x86_64(arch);
+        break;
+      default:
+        break;
+      }
+      break;
+    }
+
     default:
       break;
     }
@@ -289,7 +305,7 @@ Status ELFLinuxPrStatus::Parse(const Dat
 
   // Read field by field to correctly account for endianess of both the core
   // dump and the platform running lldb.
-  offset_t offset = 0;
+  lldb::offset_t offset = 0;
   si_signo = data.GetU32(&offset);
   si_code = data.GetU32(&offset);
   si_errno = data.GetU32(&offset);
@@ -357,7 +373,7 @@ Status ELFLinuxPrPsInfo::Parse(const Dat
     return error;
   }
   size_t size = 0;
-  offset_t offset = 0;
+  lldb::offset_t offset = 0;
 
   pr_state = data.GetU8(&offset);
   pr_sname = data.GetU8(&offset);
@@ -425,7 +441,7 @@ Status ELFLinuxSigInfo::Parse(const Data
 
   // Parsing from a 32 bit ELF core file, and populating/reusing the structure
   // properly, because the struct is for the 64 bit version
-  offset_t offset = 0;
+  lldb::offset_t offset = 0;
   si_signo = data.GetU32(&offset);
   si_code = data.GetU32(&offset);
   si_errno = data.GetU32(&offset);
