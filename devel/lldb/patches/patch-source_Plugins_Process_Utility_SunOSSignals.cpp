$NetBSD$

--- source/Plugins/Process/Utility/SunOSSignals.cpp.orig	2020-02-27 22:37:20.791293313 +0000
+++ source/Plugins/Process/Utility/SunOSSignals.cpp
@@ -0,0 +1,106 @@
+//===-- FreeBSDSignals.cpp --------------------------------------*- C++ -*-===//
+//
+//                     The LLVM Compiler Infrastructure
+//
+// This file is distributed under the University of Illinois Open Source
+// License. See LICENSE.TXT for details.
+//
+//===----------------------------------------------------------------------===//
+
+// C Includes
+// C++ Includes
+// Other libraries and framework includes
+// Project includes
+#include "SunOSSignals.h"
+
+using namespace lldb_private;
+
+SunOSSignals::SunOSSignals() : UnixSignals() { Reset(); }
+
+void SunOSSignals::Reset() {
+  m_signals.clear();
+
+  //        SIGNO   NAME  SUPPRESS STOP NOTIFY DESCRIPTION
+  AddSignal(1, "SIGHUP", false, true, true, "hangup");
+  AddSignal(2, "SIGINT", true, true, true, "interrupt");
+  AddSignal(3, "SIGQUIT", false, true, true, "quit");
+  AddSignal(4, "SIGILL", false, true, true, "illegal instruction");
+  AddSignal(5, "SIGTRAP", true, true, true,
+            "trace trap (not reset when caught)");
+  AddSignal(6, "SIGABRT", false, true, true, "abort()/IOT trap", "SIGIOT");
+  AddSignal(7, "SIGEMT", false, true, true, "emulation trap");
+  AddSignal(8, "SIGFPE", false, true, true, "floating point exception");
+  AddSignal(9, "SIGKILL", false, true, true, "kill");
+  AddSignal(10, "SIGBUS", false, true, true, "bus error");
+  AddSignal(11, "SIGSEGV", false, true, true, "segmentation violation");
+  AddSignal(12, "SIGSYS", false, true, true, "bad argument to system call");
+  AddSignal(13, "SIGPIPE", false, true, true,
+            "write to pipe with reading end closed");
+  AddSignal(14, "SIGALRM", false, false, false, "alarm");
+  AddSignal(15, "SIGTERM", false, true, true, "termination requested");
+  AddSignal(16, "SIGUSR1", false, true, true, "user defined signal 1");
+  AddSignal(17, "SIGUSR2", false, true, true, "user defined signal 2");
+  AddSignal(18, "SIGCHLD", false, false, true, "child status has changed",
+            "SIGCLD");
+  AddSignal(19, "SIGPWR", false, true, true, "power-fail restart");
+  AddSignal(20, "SIGWINCH", false, true, true, "window size change");
+  AddSignal(21, "SIGURG", false, true, true, "urgent socket condition");
+  AddSignal(22, "SIGPOLL", false, true, true, "pollable event occured",
+            "SIGIO");
+  AddSignal(23, "SIGSTOP", true, true, true, "process stop");
+  AddSignal(24, "SIGTSTP", false, true, true, "tty stop");
+  AddSignal(25, "SIGCONT", false, true, true, 
+            "stopped process has been continued");
+  AddSignal(26, "SIGTTIN", false, true, true, "background tty read attempted");
+  AddSignal(27, "SIGTTOU", false, true, true, "background tty write attempted");
+  AddSignal(28, "SIGVTALRM", false, true, true, "virtual timer expired");
+  AddSignal(29, "SIGPROF", false, false, false, "profiling timer expired");
+  AddSignal(30, "SIGXCPU", false, true, true, "exceeded cpu limit");
+  AddSignal(31, "SIGXFSZ", false, true, true, "exceeded file size limit");
+  AddSignal(32, "SIGWAITING", false, true, true, 
+            "reserved signal no longer used by threading code");
+  AddSignal(33, "SIGLWP", false, true, true,
+            "reserved signal no longer used by threading code");
+  AddSignal(34, "SIGFREEZE", false, true, true, "special signal used by CPR");
+  AddSignal(35, "SIGTHAW", false, true, true, "special used by CPR");
+  AddSignal(36, "SIGCANCEL", false, true, true,
+            "reserved signal for thread cancellation");
+  AddSignal(37, "SIGLOST", false, true, true, "resource lost (eg, record-lock lost)");
+  AddSignal(38, "SIGXRES", false, true, true, "resource control exceeded");
+  AddSignal(39, "SIGJVM1", false, true, true, "reserved signal for Java Virtual Machine");
+  AddSignal(40, "SIGJVM2", false, true, true, "reserved signal for Java Virtual Machine");
+  AddSignal(41, "SIGINFO", false, true, true, "information request");
+  AddSignal(42, "SIGRTMIN", false, false, false, "real time signal 0");
+  AddSignal(43, "SIGTMIN+1", false, false, false, "real time signal 1");
+  AddSignal(44, "SIGTMIN+2", false, false, false, "real time signal 2");
+  AddSignal(45, "SIGTMIN+3", false, false, false, "real time signal 3");
+  AddSignal(46, "SIGTMIN+4", false, false, false, "real time signal 4");
+  AddSignal(47, "SIGTMIN+5", false, false, false, "real time signal 5");
+  AddSignal(48, "SIGTMIN+6", false, false, false, "real time signal 6");
+  AddSignal(49, "SIGTMIN+7", false, false, false, "real time signal 7");
+  AddSignal(50, "SIGTMIN+8", false, false, false, "real time signal 8");
+  AddSignal(51, "SIGTMIN+9", false, false, false, "real time signal 9");
+  AddSignal(52, "SIGTMIN+10", false, false, false, "real time signal 10");
+  AddSignal(53, "SIGTMIN+11", false, false, false, "real time signal 11");
+  AddSignal(54, "SIGTMIN+12", false, false, false, "real time signal 12");
+  AddSignal(55, "SIGTMIN+13", false, false, false, "real time signal 13");
+  AddSignal(56, "SIGTMIN+14", false, false, false, "real time signal 14");
+  AddSignal(57, "SIGTMIN+15", false, false, false, "real time signal 15");
+  AddSignal(58, "SIGTMIN+16", false, false, false, "real time signal 16");
+  AddSignal(59, "SIGTMAX-15", false, false, false, "real time signal 17");
+  AddSignal(60, "SIGTMAX-14", false, false, false, "real time signal 18");
+  AddSignal(61, "SIGTMAX-13", false, false, false, "real time signal 19");
+  AddSignal(62, "SIGTMAX-12", false, false, false, "real time signal 20");
+  AddSignal(63, "SIGTMAX-11", false, false, false, "real time signal 21");
+  AddSignal(64, "SIGTMAX-10", false, false, false, "real time signal 22");
+  AddSignal(65, "SIGTMAX-9", false, false, false, "real time signal 23");
+  AddSignal(66, "SIGTMAX-8", false, false, false, "real time signal 24");
+  AddSignal(67, "SIGTMAX-7", false, false, false, "real time signal 25");
+  AddSignal(68, "SIGTMAX-6", false, false, false, "real time signal 26");
+  AddSignal(69, "SIGTMAX-5", false, false, false, "real time signal 27");
+  AddSignal(70, "SIGTMAX-4", false, false, false, "real time signal 28");
+  AddSignal(71, "SIGTMAX-3", false, false, false, "real time signal 29");
+  AddSignal(72, "SIGTMAX-2", false, false, false, "real time signal 30");
+  AddSignal(73, "SIGTMAX-1", false, false, false, "real time signal 31");
+  AddSignal(74, "SIGTMAX", false, false, false, "real time signal 32");
+}
