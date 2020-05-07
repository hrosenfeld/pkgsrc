$NetBSD$

--- source/Plugins/Process/elf-core/RegisterContextPOSIXCore_x86_64.cpp.orig	2019-01-19 08:50:56.000000000 +0000
+++ source/Plugins/Process/elf-core/RegisterContextPOSIXCore_x86_64.cpp
@@ -26,12 +26,62 @@ RegisterContextCorePOSIX_x86_64::Registe
   if (len != size)
     m_gpregset.reset();
 
-  DataExtractor fpregset = getRegset(
-      notes, register_info->GetTargetArchitecture().GetTriple(), FPR_Desc);
+  ArchSpec arch = register_info->GetTargetArchitecture();
   size = sizeof(FXSAVE);
   m_fpregset.reset(new uint8_t[size]);
-  len =
+
+  if (arch.GetTriple().getOS() == llvm::Triple::Solaris) {
+    if (arch.GetMachine() == llvm::Triple::x86_64) {
+      len = gpregset.ExtractBytes(
+        SUNOS::GPR_LEN_x86_64, size, lldb::eByteOrderLittle, m_fpregset.get());
+    } else {
+      // Solaris/illumos 32bit x86 cores use an FSAVE structure plus some extra
+      // fields for XMM registers.
+      struct {
+        uint32_t cw, sw, tag, ipoff, cssel, dataoff, datasel;
+        uint16_t st[8][5];
+        uint32_t status, mxcsr, xstatus, __pad[2];
+        uint32_t xmm[8][4];
+      } fpstate = {0};
+
+      std::fill_n(m_fpregset.get(), size, 0);
+
+      FXSAVE *fxstate = (FXSAVE *)m_fpregset.get();
+
+      len = gpregset.ExtractBytes(
+        SUNOS::GPR_LEN_x86, sizeof (fpstate), lldb::eByteOrderLittle, &fpstate);
+
+      int i;
+      int8_t ftag = 0;
+
+      for (i = 3; i != 0x30000; i<<=2, ftag >>= 1)
+        ftag |= ((fpstate.tag & i) != 3 ? 0x80 : 0);
+
+      fxstate->ftag = ftag;
+
+      fxstate->fctrl = (uint16_t)fpstate.cw;
+      fxstate->fstat = (uint16_t)fpstate.sw;
+      fxstate->ptr.i386_.fioff = fpstate.ipoff;
+      fxstate->ptr.i386_.fiseg = fpstate.cssel & 0xffff;
+      fxstate->fop = (fpstate.cssel >> 16) & 0x7ff;
+      fxstate->ptr.i386_.fooff = fpstate.dataoff;
+      fxstate->ptr.i386_.foseg = fpstate.datasel;
+      fxstate->mxcsr = fpstate.mxcsr;
+
+      for (int i = 0; i != 8; i++) {
+        ::memcpy(&fxstate->stmm[i], &fpstate.st[i], sizeof (fxstate->stmm[i]));
+        ::memcpy(&fxstate->xmm[i], &fpstate.xmm[i], sizeof (fxstate->xmm[i]));
+      }
+
+      len = sizeof (*fxstate);
+    }
+  } else {
+    DataExtractor fpregset = getRegset(
+      notes, register_info->GetTargetArchitecture().GetTriple(), FPR_Desc);
+    len =
       fpregset.ExtractBytes(0, size, lldb::eByteOrderLittle, m_fpregset.get());
+  }
+
   if (len != size)
     m_fpregset.reset();
 }
