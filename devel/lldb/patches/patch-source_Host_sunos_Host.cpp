$NetBSD$

--- source/Host/sunos/Host.cpp.orig	2020-04-01 11:49:39.176658565 +0000
+++ source/Host/sunos/Host.cpp
@@ -0,0 +1,218 @@
+//===-- source/Host/sunos/Host.cpp ------------------------------*- C++ -*-===//
+//
+// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
+// See https://llvm.org/LICENSE.txt for license information.
+// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
+//
+//===----------------------------------------------------------------------===//
+
+#include <sys/types.h>
+#include <libproc.h>
+#include <unistd.h>
+
+#include "lldb/Utility/ProcessInfo.h"
+
+#include "lldb/Host/Host.h"
+#include "lldb/Host/HostInfo.h"
+
+#include "llvm/Support/Host.h"
+
+extern "C" {
+	extern char **environ;
+}
+
+using namespace lldb_private;
+
+struct proc_walk_info {
+	const ProcessInstanceInfoMatch &match_info;
+	ProcessInstanceInfoList &process_infos;
+	pid_t our_pid;
+	uid_t our_uid;
+	bool all_users;
+};
+
+void
+proc_get_args(struct ps_prochandle *pr, ProcessInstanceInfo &pi,
+    const psinfo_t *ps)
+{
+	uint64_t argv[ps->pr_argc];
+	ssize_t ret;
+	int i;
+
+	/* Read argv from the process. Convert 32bit pointers if necessary. */
+	if (ps->pr_dmodel == PR_MODEL_LP64) {
+		ret = Pread(pr, argv, ps->pr_argc * sizeof (uint64_t),
+		    ps->pr_argv);
+	} else if (ps->pr_dmodel == PR_MODEL_ILP32) {
+		uint32_t argv32[ps->pr_argc];
+
+		ret = Pread(pr, argv32, ps->pr_argc * sizeof (uint32_t),
+		    ps->pr_argv);
+
+		if (ret > 0)
+			for (i = 0; i != ps->pr_argc; i++)
+				argv[i] = argv32[i];
+	} else {
+		return;
+	}
+
+	if (ret <= 0)
+		return;
+
+	/* Read argument strings, one by one. */
+	for (i = 0; i != ps->pr_argc; i++) {
+		size_t alen = 128;
+		char *arg = new char[alen];
+
+		/*
+		 * Repeatedly read the argument string until it fits. Double
+		 * buffer size if it was too small.
+		 */
+		for (;;) {
+			if (Pread_string(pr, arg, alen, argv[i]) < 0) {
+				delete [] arg;
+				return;
+			}
+
+			if (strlen(arg) < alen - 1)
+				break;
+
+			if (alen >= sysconf(_SC_ARG_MAX)) {
+				delete [] arg;
+				return;
+			}
+
+			delete [] arg;
+			alen *= 2;
+			arg = new char[alen];
+		}
+
+		pi.GetArguments().AppendArgument(arg);
+
+		delete [] arg;
+	}
+}
+
+int
+proc_env_cb(void *arg, struct ps_prochandle *pr, uintptr_t envp,
+    const char *var)
+{
+	ProcessInstanceInfo *pi = (ProcessInstanceInfo *)arg;
+
+	pi->GetEnvironment().insert(var);
+
+	return (0);
+}
+
+void
+proc_gather_info(ProcessInstanceInfo &pi, struct ps_prochandle *pr,
+    const psinfo_t *ps)
+{
+	char path[PATH_MAX];
+
+	pi.Clear();
+
+	if (ps->pr_dmodel == PR_MODEL_ILP32) {
+		pi.SetArchitecture(
+		    HostInfo::GetArchitecture(HostInfo::eArchKind32));
+	} else if (ps->pr_dmodel == PR_MODEL_LP64) {
+		pi.SetArchitecture(
+		    HostInfo::GetArchitecture(HostInfo::eArchKind64));
+	} else {
+		return;
+	}
+
+	pi.SetProcessID(ps->pr_pid);
+	pi.SetParentProcessID(ps->pr_ppid);
+	pi.SetUserID(ps->pr_uid);
+	pi.SetEffectiveUserID(ps->pr_euid);
+	pi.SetGroupID(ps->pr_gid);
+	pi.SetEffectiveGroupID(ps->pr_egid);
+
+	if (Pexecname(pr, path, sizeof (path)) != NULL)
+		pi.GetExecutableFile().SetFile(path, FileSpec::Style::native);
+
+	(void) Penv_iter(pr, proc_env_cb, &pi);
+
+	proc_get_args(pr, pi, ps);
+}
+
+int
+proc_walk_cb(psinfo_t *ps, lwpsinfo_t *lwps, void *arg)
+{
+	struct proc_walk_info *pwi = (struct proc_walk_info *)arg;
+	struct ps_prochandle *pr;
+	ProcessInstanceInfo pi;
+	int err;
+
+	/* Skip ourselves. */
+	if (pwi->our_pid == ps->pr_pid)
+		return (0);
+
+	/* Skip other users' processes unless requested otherwise. */
+	if (!pwi->all_users && (pwi->our_uid != ps->pr_uid))
+		return (0);
+
+	/* Skip processes that have no active lwps. */
+	if (ps->pr_nlwp == 0)
+		return (0);
+
+	/* Skip zombies. */
+	if (ps->pr_lwp.pr_sname == 'Z')
+		return (0);
+
+	if ((pr = Pgrab(ps->pr_pid, PGRAB_RDONLY, &err)) == NULL)
+		return (0);
+
+	proc_gather_info(pi, pr, ps);
+	Prelease(pr, 0);
+
+	if (pwi->match_info.Matches(pi))
+		pwi->process_infos.Append(pi);
+
+	return (0);
+}
+
+uint32_t
+Host::FindProcesses(const ProcessInstanceInfoMatch &match_info,
+    ProcessInstanceInfoList &process_infos)
+{
+	struct proc_walk_info pwi = {
+		.match_info = match_info,
+		.process_infos = process_infos,
+		.our_pid = getpid(),
+		.our_uid = getuid(),
+		.all_users = match_info.GetMatchAllUsers() || (pwi.our_uid == 0)
+	};
+
+	(void) proc_walk(proc_walk_cb, &pwi, PR_WALK_PROC);
+
+	return (process_infos.GetSize());
+}
+
+bool
+Host::GetProcessInfo(lldb::pid_t pid, ProcessInstanceInfo &process_info)
+{
+	struct ps_prochandle *pr;
+	int err;
+
+	if ((pr = Pgrab(pid, PGRAB_RDONLY, &err)) == NULL)
+		return (false);
+
+	proc_gather_info(process_info, pr, Ppsinfo(pr));
+	Prelease(pr, 0);
+
+	return (true);
+}
+
+Environment
+Host::GetEnvironment()
+{
+	return Environment(environ);
+}
+
+Status
+Host::ShellExpandArguments(ProcessLaunchInfo &launch_info)
+{
+	return Status("unimplemented");
+}
