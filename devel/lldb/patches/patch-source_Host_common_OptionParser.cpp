$NetBSD$

--- source/Host/common/OptionParser.cpp.orig	2019-07-10 17:09:47.000000000 +0000
+++ source/Host/common/OptionParser.cpp
@@ -17,7 +17,7 @@ using namespace lldb_private;
 void OptionParser::Prepare(std::unique_lock<std::mutex> &lock) {
   static std::mutex g_mutex;
   lock = std::unique_lock<std::mutex>(g_mutex);
-#ifdef __GLIBC__
+#if defined(__GLIBC__) || defined(__sun)
   optind = 0;
 #else
   optreset = 1;
@@ -35,7 +35,7 @@ int OptionParser::Parse(llvm::MutableArr
     option opt;
     opt.flag = longopts->flag;
     opt.val = longopts->val;
-    opt.name = longopts->definition->long_option;
+    opt.name = (char *)longopts->definition->long_option;
     opt.has_arg = longopts->definition->option_has_arg;
     opts.push_back(opt);
     ++longopts;
