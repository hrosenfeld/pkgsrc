$NetBSD$

--- cmake/modules/LLDBConfig.cmake.orig	2019-08-26 12:36:40.000000000 +0000
+++ cmake/modules/LLDBConfig.cmake
@@ -397,9 +397,13 @@ endif()
 
 list(APPEND system_libs ${CMAKE_DL_LIBS})
 
+if (CMAKE_SYSTEM_NAME MATCHES "SunOS")
+  list(APPEND system_libs "-lsocket -lnsl")
+endif()
+
 # Figure out if lldb could use lldb-server.  If so, then we'll
 # ensure we build lldb-server when an lldb target is being built.
-if (CMAKE_SYSTEM_NAME MATCHES "Android|Darwin|FreeBSD|Linux|NetBSD")
+if (CMAKE_SYSTEM_NAME MATCHES "Android|Darwin|FreeBSD|Linux|NetBSD|SunOS")
   set(LLDB_CAN_USE_LLDB_SERVER 1)
 else()
   set(LLDB_CAN_USE_LLDB_SERVER 0)
