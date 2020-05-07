$NetBSD$

--- tools/lldb-server/lldb-platform.cpp.orig	2019-05-23 11:14:47.000000000 +0000
+++ tools/lldb-server/lldb-platform.cpp
@@ -154,7 +154,7 @@ int main_platform(int argc, char *argv[]
 
   std::string short_options(OptionParser::GetShortOptionString(g_long_options));
 
-#if __GLIBC__
+#if defined(__GLIBC__) || defined(__sun)
   optind = 0;
 #else
   optreset = 1;
@@ -318,6 +318,10 @@ int main_platform(int argc, char *argv[]
         // Parent will continue to listen for new connections.
         continue;
       } else {
+        // We lose the logging on the child side of the fork, re-enable it.  
+        if (!LLDBServerUtilities::SetupLogging(log_file, log_channels, 0))
+          return -1;
+
         // Child process will handle the connection and exit.
         g_server = 0;
         // Listening socket is owned by parent process.
