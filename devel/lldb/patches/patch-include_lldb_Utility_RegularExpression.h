$NetBSD$

--- include/lldb/Utility/RegularExpression.h.orig	2019-04-10 20:48:55.000000000 +0000
+++ include/lldb/Utility/RegularExpression.h
@@ -66,7 +66,8 @@ public:
 
     void Clear() {
       const size_t num_matches = m_matches.size();
-      regmatch_t invalid_match = {-1, -1};
+      regmatch_t invalid_match = { 0 };
+      invalid_match.rm_so = invalid_match.rm_eo = -1;
       for (size_t i = 0; i < num_matches; ++i)
         m_matches[i] = invalid_match;
     }
