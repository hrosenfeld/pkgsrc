$NetBSD$

--- source/Core/FormatEntity.cpp.orig	2019-06-17 19:53:11.000000000 +0000
+++ source/Core/FormatEntity.cpp
@@ -1210,7 +1210,8 @@ bool FormatEntity::Format(const Entry &e
                                               : llvm::Triple::UnknownOS;
             if ((ostype == llvm::Triple::FreeBSD) ||
                 (ostype == llvm::Triple::Linux) ||
-                (ostype == llvm::Triple::NetBSD)) {
+                (ostype == llvm::Triple::NetBSD) ||
+                (ostype == llvm::Triple::Solaris)) {
               format = "%" PRIu64;
             }
           } else {
