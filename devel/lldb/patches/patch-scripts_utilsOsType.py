$NetBSD$

--- scripts/utilsOsType.py.orig	2018-10-04 20:34:58.000000000 +0000
+++ scripts/utilsOsType.py
@@ -38,6 +38,7 @@ if sys.version_info.major >= 3:
         OpenBSD = 5
         Windows = 6
         kFreeBSD = 7
+        SunOS = 8
 else:
     class EnumOsType(object):
         values = ["Unknown",
@@ -47,7 +48,8 @@ else:
                   "NetBSD",
                   "OpenBSD",
                   "Windows",
-                  "kFreeBSD"]
+                  "kFreeBSD",
+                  "SunOS"]
 
         class __metaclass__(type):
             #++----------------------------------------------------------------
@@ -99,5 +101,7 @@ def determine_os_type():
         eOSType = EnumOsType.Windows
     elif strOS.startswith("gnukfreebsd"):
         eOSType = EnumOsType.kFreeBSD
+    elif strOS == "sunos5":
+        eOSType = EnumOsType.SunOS
 
     return eOSType
