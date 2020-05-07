$NetBSD$

--- include/lldb/Host/sunos/HostInfoSunOS.h.orig	2020-04-01 11:32:26.237291651 +0000
+++ include/lldb/Host/sunos/HostInfoSunOS.h
@@ -0,0 +1,29 @@
+//===-- HostInfoSunOS.h -----------------------------------------*- C++ -*-===//
+//
+//		     The LLVM Compiler Infrastructure
+//
+// This file is distributed under the University of Illinois Open Source
+// License. See LICENSE.TXT for details.
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef lldb_Host_sunos_HostInfoSunOS_h_
+#define lldb_Host_sunos_HostInfoSunOS_h_
+
+#include "lldb/Host/posix/HostInfoPosix.h"
+#include "lldb/Utility/FileSpec.h"
+#include "llvm/Support/VersionTuple.h"
+
+namespace lldb_private {
+
+class HostInfoSunOS : public HostInfoPosix {
+public:
+	static llvm::VersionTuple GetOSVersion();
+	static bool GetOSBuildString(std::string &s);
+	static bool GetOSKernelDescription(std::string &s);
+	static FileSpec GetProgramFileSpec();
+};
+
+}
+
+#endif
