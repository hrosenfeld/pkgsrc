$NetBSD$

--- source/Target/UnixSignals.cpp.orig	2019-04-10 20:48:55.000000000 +0000
+++ source/Target/UnixSignals.cpp
@@ -11,6 +11,7 @@
 #include "Plugins/Process/Utility/LinuxSignals.h"
 #include "Plugins/Process/Utility/MipsLinuxSignals.h"
 #include "Plugins/Process/Utility/NetBSDSignals.h"
+#include "Plugins/Process/Utility/SunOSSignals.h"
 #include "lldb/Host/HostInfo.h"
 #include "lldb/Host/StringConvert.h"
 #include "lldb/Utility/ArchSpec.h"
@@ -46,6 +47,8 @@ lldb::UnixSignalsSP UnixSignals::Create(
     return std::make_shared<FreeBSDSignals>();
   case llvm::Triple::NetBSD:
     return std::make_shared<NetBSDSignals>();
+  case llvm::Triple::Solaris:
+    return std::make_shared<SunOSSignals>();
   default:
     return std::make_shared<UnixSignals>();
   }
