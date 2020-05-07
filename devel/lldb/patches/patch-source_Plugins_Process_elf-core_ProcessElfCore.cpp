$NetBSD$

--- source/Plugins/Process/elf-core/ProcessElfCore.cpp.orig	2019-06-11 20:16:13.000000000 +0000
+++ source/Plugins/Process/elf-core/ProcessElfCore.cpp
@@ -846,6 +846,106 @@ llvm::Error ProcessElfCore::parseLinuxNo
   return llvm::Error::success();
 }
 
+/// A Solaris / illumos core file usually contains two PT_NOTE segments: an
+/// old-style one for binary compatibility with old tools, which we'll ignore,
+/// and the new-style one which we're interested.
+///
+/// The new-style PT_NOTE segment starts with a few per-process entries,
+/// followed by per-thread entries, starting with NT_LWPSINFO for each thread.
+llvm::Error ProcessElfCore::parseSunOSNotes(llvm::ArrayRef<CoreNote> notes) {
+  bool new_style = false;
+  bool in_thread = false;
+  lldb::offset_t offset;
+  size_t len;
+  ThreadData thread_data;
+  Status status;
+
+  for (const auto &note: notes) {
+    if (note.info.n_type == SUNOS::NT_PRPSINFO ||
+        note.info.n_type == SUNOS::NT_PRSTATUS ||
+        note.info.n_type == SUNOS::NT_PRFPREG) {
+      if (new_style) {
+        status.SetErrorString("unexpected old-style PT_NOTE entry");
+        return status.ToError();
+      } else {
+        return llvm::Error::success();
+      }
+    }
+
+    new_style = true;
+
+    switch (note.info.n_type) {
+    case SUNOS::NT_AUXV:
+      m_auxv = note.data;
+      break;
+
+    case SUNOS::NT_LWPSINFO:
+      // Save the previous thread and start a new one.
+      if (in_thread) {
+        m_thread_data.push_back(thread_data);
+        thread_data = ThreadData();
+        in_thread = false;
+      }
+
+      offset = 4; // skip pr_flag
+      thread_data.tid = note.data.GetU32(&offset);
+
+      in_thread = true;
+      break;
+
+    case SUNOS::NT_LWPSTATUS:
+      if (!in_thread)
+        goto outoforder;
+
+      offset = 4; // skip pr_flags
+      if (thread_data.tid != note.data.GetU32(&offset))
+        goto id_mismatch;
+
+      offset += 4; // skip pr_why & pr_what
+      thread_data.signo = thread_data.prstatus_sig = note.data.GetU16(&offset);
+
+      offset = 0;
+      switch (GetArchitecture().GetMachine()) {
+      case  llvm::Triple::x86_64:
+        offset = SUNOS::GPR_OFF_x86_64;
+        len = SUNOS::GPR_LEN_x86_64 + SUNOS::FPR_LEN_x86_64;
+        break;
+      case llvm::Triple::x86:
+        offset = SUNOS::GPR_OFF_x86;
+        len = SUNOS::GPR_LEN_x86 + SUNOS::FPR_LEN_x86_64;
+        break;
+      }
+      assert(offset != 0);
+      thread_data.gpregset = DataExtractor(note.data, offset, len);
+      break;
+
+    case SUNOS::NT_LWPNAME:
+      if (!in_thread)
+        goto outoforder;
+
+      offset = 0;
+      if (thread_data.tid != note.data.GetU64(&offset))
+        goto id_mismatch;
+
+      thread_data.name = note.data.GetCStr(&offset, 32);
+      break;
+
+    default:
+      thread_data.notes.push_back(note);
+    }
+  }
+  m_thread_data.push_back(thread_data);
+  return llvm::Error::success();
+
+id_mismatch:
+  status.SetErrorString("LWP id mismatch");
+  return status.ToError();
+
+outoforder:
+  status.SetErrorString("unexpected order of PT_NOTE entries");
+  return status.ToError();
+}
+
 /// Parse Thread context from PT_NOTE segment and store it in the thread list
 /// A note segment consists of one or more NOTE entries, but their types and
 /// meaning differ depending on the OS.
@@ -865,6 +965,8 @@ llvm::Error ProcessElfCore::ParseThreadC
     return parseNetBSDNotes(*notes_or_error);
   case llvm::Triple::OpenBSD:
     return parseOpenBSDNotes(*notes_or_error);
+  case llvm::Triple::Solaris:
+    return parseSunOSNotes(*notes_or_error);
   default:
     return llvm::make_error<llvm::StringError>(
         "Don't know how to parse core file. Unsupported OS.",
