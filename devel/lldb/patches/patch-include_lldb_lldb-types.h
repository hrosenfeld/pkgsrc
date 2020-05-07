$NetBSD$

--- include/lldb/lldb-types.h.orig	2019-04-10 20:48:55.000000000 +0000
+++ include/lldb/lldb-types.h
@@ -47,6 +47,28 @@ typedef thread_result_t (*thread_func_t)
 typedef void *pipe_t;                             // Host pipe type is HANDLE
 } // namespace lldb
 
+#elif defined(__sun)
+
+#include <pthread.h>
+
+// don't want to include libproc.h here, so use a forward declaration
+extern "C" {
+  struct ps_prochandle;
+}
+
+namespace lldb {
+// SunOS Types - same as MacOSX except for process_t
+typedef pthread_rwlock_t rwlock_t;
+typedef struct ps_prochandle *process_t; // Process type is libproc handle
+typedef pthread_t thread_t; // Host thread type
+typedef int file_t;         // Host file type
+typedef int socket_t;       // Host socket type
+typedef void *thread_arg_t;             // Host thread argument type
+typedef void *thread_result_t;          // Host thread result type
+typedef void *(*thread_func_t)(void *); // Host thread function type
+typedef int pipe_t;                     // Host pipe type
+} // namespace lldb
+
 #else
 
 #include <pthread.h>
