$NetBSD$

--- source/Host/sunos/HostInfoSunOS.cpp.orig	2020-04-01 11:32:26.832104388 +0000
+++ source/Host/sunos/HostInfoSunOS.cpp
@@ -0,0 +1,72 @@
+//===-- HostInfoSunOS.cpp ---------------------------------------*- C++ -*-===//
+//
+//		     The LLVM Compiler Infrastructure
+//
+// This file is distributed under the University of Illinois Open Source
+// License. See LICENSE.TXT for details.
+//
+//===----------------------------------------------------------------------===//
+
+#include "lldb/Host/sunos/HostInfoSunOS.h"
+
+#include <unistd.h>
+#include <sys/utsname.h>
+
+using namespace lldb_private;
+
+llvm::VersionTuple
+HostInfoSunOS::GetOSVersion()
+{
+	unsigned maj, min;
+	struct utsname un;
+
+	::memset(&un, 0, sizeof (utsname));
+	if (uname(&un) < 0)
+		return (llvm::VersionTuple());
+
+	if (sscanf(un.release, "%u.%u", &maj, &min) != 2)
+		return (llvm::VersionTuple());
+
+	return (llvm::VersionTuple(maj, min));
+}
+
+bool
+HostInfoSunOS::GetOSBuildString(std::string &s)
+{
+	struct utsname un;
+
+	::memset(&un, 0, sizeof (utsname));
+	if (uname(&un) < 0)
+		return (false);
+
+	s.assign(un.version);
+	return (true);
+}
+
+bool
+HostInfoSunOS::GetOSKernelDescription(std::string &s)
+{
+	return (GetOSBuildString(s));
+}
+
+FileSpec
+HostInfoSunOS::GetProgramFileSpec()
+{
+	static FileSpec g_program_filespec;
+	char path[PATH_MAX];
+	ssize_t len;
+
+	if (g_program_filespec)
+		goto out;
+
+	len = readlink("/proc/self/path/a.out", path, sizeof (path) - 1);
+
+	if (len < 0)
+		goto out;
+
+	path[len] = '\0';
+	g_program_filespec.SetFile(path, FileSpec::Style::native);
+
+out:
+	return (g_program_filespec);
+}
