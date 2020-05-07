$NetBSD$

--- /dev/null	2018-05-01 04:59:11.000000000 +0000
+++ source/Plugins/Process/Utility/SunOSSignals.h
@@ -0,0 +1,29 @@
+//===-- SunOSSignals.h ------------------------------------------*- C++ -*-===//
+//
+//                     The LLVM Compiler Infrastructure
+//
+// This file is distributed under the University of Illinois Open Source
+// License. See LICENSE.TXT for details.
+//
+//===----------------------------------------------------------------------===//
+
+#ifndef liblldb_SunOSSignals_H_
+#define liblldb_SunOSSignals_H_
+
+// Project includes
+#include "lldb/Target/UnixSignals.h"
+
+namespace lldb_private {
+
+/// SunOS specific set of Unix signals.
+class SunOSSignals : public UnixSignals {
+public:
+  SunOSSignals();
+
+private:
+  void Reset() override;
+};
+
+} // namespace lldb_private
+
+#endif // liblldb_SunOSSignals_H_
