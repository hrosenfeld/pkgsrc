$NetBSD$

--- source/API/SystemInitializerFull.cpp.orig	2019-06-24 18:21:05.000000000 +0000
+++ source/API/SystemInitializerFull.cpp
@@ -76,6 +76,7 @@
 #include "Plugins/Platform/MacOSX/PlatformRemoteiOS.h"
 #include "Plugins/Platform/NetBSD/PlatformNetBSD.h"
 #include "Plugins/Platform/OpenBSD/PlatformOpenBSD.h"
+#include "Plugins/Platform/SunOS/PlatformSunOS.h"
 #include "Plugins/Platform/Windows/PlatformWindows.h"
 #include "Plugins/Platform/gdb-server/PlatformRemoteGDBServer.h"
 #include "Plugins/Process/elf-core/ProcessElfCore.h"
@@ -157,6 +158,7 @@ llvm::Error SystemInitializerFull::Initi
   platform_linux::PlatformLinux::Initialize();
   platform_netbsd::PlatformNetBSD::Initialize();
   platform_openbsd::PlatformOpenBSD::Initialize();
+  platform_sunos::PlatformSunOS::Initialize();
   PlatformWindows::Initialize();
   platform_android::PlatformAndroid::Initialize();
   PlatformRemoteiOS::Initialize();
@@ -372,6 +374,7 @@ void SystemInitializerFull::Terminate()
   platform_linux::PlatformLinux::Terminate();
   platform_netbsd::PlatformNetBSD::Terminate();
   platform_openbsd::PlatformOpenBSD::Terminate();
+  platform_sunos::PlatformSunOS::Terminate();
   PlatformWindows::Terminate();
   platform_android::PlatformAndroid::Terminate();
   PlatformMacOSX::Terminate();
