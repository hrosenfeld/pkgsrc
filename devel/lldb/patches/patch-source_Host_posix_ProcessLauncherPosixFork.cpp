$NetBSD$

--- source/Host/posix/ProcessLauncherPosixFork.cpp.orig	2019-05-13 04:42:32.000000000 +0000
+++ source/Host/posix/ProcessLauncherPosixFork.cpp
@@ -16,7 +16,11 @@
 #include "llvm/Support/Errno.h"
 
 #include <limits.h>
+#ifdef __sun
+#include <libproc.h>
+#else
 #include <sys/ptrace.h>
+#endif
 #include <sys/wait.h>
 #include <unistd.h>
 
@@ -88,8 +92,8 @@ static void DupDescriptor(int error_fd,
   return;
 }
 
-static void LLVM_ATTRIBUTE_NORETURN ChildFunc(int error_fd,
-                                              const ProcessLaunchInfo &info) {
+static void
+ChildFuncHelper(int error_fd, const ProcessLaunchInfo &info) {
   if (info.GetFlags().Test(eLaunchFlagLaunchInSeparateProcessGroup)) {
     if (setpgid(0, 0) != 0)
       ExitWithError(error_fd, "setpgid");
@@ -115,17 +119,12 @@ static void LLVM_ATTRIBUTE_NORETURN Chil
     }
   }
 
-  const char **argv = info.GetArguments().GetConstArgumentVector();
-
   // Change working directory
   if (info.GetWorkingDirectory() &&
       0 != ::chdir(info.GetWorkingDirectory().GetCString()))
     ExitWithError(error_fd, "chdir");
 
   DisableASLRIfRequested(error_fd, info);
-  Environment env = info.GetEnvironment();
-  FixupEnvironment(env);
-  Environment::Envp envp = env.getEnvp();
 
   // Clear the signal mask to prevent the child from being affected by any
   // masking done by the parent.
@@ -147,10 +146,24 @@ static void LLVM_ATTRIBUTE_NORETURN Chil
       if (!info.GetFileActionForFD(fd) && fd != error_fd)
         close(fd);
 
+#ifndef __sun
     // Start tracing this child that is about to exec.
     if (ptrace(PT_TRACE_ME, 0, nullptr, 0) == -1)
       ExitWithError(error_fd, "ptrace");
+#endif
   }
+}
+
+static void LLVM_ATTRIBUTE_NORETURN ChildFunc(int error_fd,
+                                              const ProcessLaunchInfo &info) {
+
+  ChildFuncHelper(error_fd, info);
+
+  const char **argv = info.GetArguments().GetConstArgumentVector();
+
+  Environment env = info.GetEnvironment();
+  FixupEnvironment(env);
+  Environment::Envp envp = env.getEnvp();
 
   // Execute.  We should never return...
   execve(argv[0], const_cast<char *const *>(argv), envp);
@@ -175,6 +188,17 @@ static void LLVM_ATTRIBUTE_NORETURN Chil
   ExitWithError(error_fd, "execve");
 }
 
+#ifdef __sun
+static int efd;
+static const ProcessLaunchInfo *linfo;
+
+extern "C" void
+Pcreate_callback(struct ps_prochandle *pr)
+{
+  ChildFuncHelper(efd, *linfo);
+}
+#endif
+
 HostProcess
 ProcessLauncherPosixFork::LaunchProcess(const ProcessLaunchInfo &launch_info,
                                         Status &error) {
@@ -188,6 +212,7 @@ ProcessLauncherPosixFork::LaunchProcess(
   if (error.Fail())
     return HostProcess();
 
+#ifndef __sun
   ::pid_t pid = ::fork();
   if (pid == -1) {
     // Fork failed
@@ -200,15 +225,46 @@ ProcessLauncherPosixFork::LaunchProcess(
     pipe.CloseReadFileDescriptor();
     ChildFunc(pipe.ReleaseWriteFileDescriptor(), launch_info);
   }
+#else
+  struct ps_prochandle *pr;
+  int err;
 
-  // parent process
+  const char **argv = launch_info.GetArguments().GetConstArgumentVector();
+  Environment env = launch_info.GetEnvironment();
+
+  FixupEnvironment(env);
+  Environment::Envp envp = env.getEnvp();
+
+  efd = pipe.GetWriteFileDescriptor();
+  linfo = &launch_info;
 
+  pr = Pxcreate(argv[0], const_cast<char *const *>(argv), envp, &err, NULL, 0);
+
+  if (pr == NULL) {
+    error.SetErrorString(Pcreate_error(err));
+    return HostProcess(LLDB_INVALID_PROCESS_ID);
+  }
+
+  const pstatus_t *ps = Pstatus(pr);
+  ::pid_t pid = ps->pr_pid;
+
+  // If we're not debugging, release the process and let it run.
+  if (!launch_info.GetFlags().Test(eLaunchFlagDebug))
+    Prelease(pr, PRELEASE_CLEAR);
+#endif
+  // parent process
   pipe.CloseWriteFileDescriptor();
   char buf[1000];
   int r = read(pipe.GetReadFileDescriptor(), buf, sizeof buf);
 
-  if (r == 0)
-    return HostProcess(pid); // No error. We're done.
+  // No error. We're done.
+  if (r == 0) {
+#ifdef __sun
+    if (launch_info.GetFlags().Test(eLaunchFlagDebug))
+      return HostProcess(pr);
+#endif
+    return HostProcess(pid);
+  }
 
   error.SetErrorString(buf);
 
