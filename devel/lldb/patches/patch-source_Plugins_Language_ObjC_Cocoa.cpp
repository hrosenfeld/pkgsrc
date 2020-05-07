$NetBSD$

--- source/Plugins/Language/ObjC/Cocoa.cpp.orig	2019-07-15 22:56:12.000000000 +0000
+++ source/Plugins/Language/ObjC/Cocoa.cpp
@@ -1102,8 +1102,10 @@ time_t lldb_private::formatters::GetOSXE
     tm_epoch.tm_mday = 1;
     tm_epoch.tm_year = 2001 - 1900;
     tm_epoch.tm_isdst = -1;
+#ifndef __sun
     tm_epoch.tm_gmtoff = 0;
     tm_epoch.tm_zone = nullptr;
+#endif
     epoch = timegm(&tm_epoch);
 #endif
   }
