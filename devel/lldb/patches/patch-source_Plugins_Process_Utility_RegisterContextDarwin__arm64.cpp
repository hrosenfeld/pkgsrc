$NetBSD$

--- source/Plugins/Process/Utility/RegisterContextDarwin_arm64.cpp.orig	2019-07-01 11:09:15.000000000 +0000
+++ source/Plugins/Process/Utility/RegisterContextDarwin_arm64.cpp
@@ -384,7 +384,7 @@ bool RegisterContextDarwin_arm64::ReadRe
     if (process_sp.get()) {
       DataExtractor regdata(&gpr.x[reg - gpr_w0], 8, process_sp->GetByteOrder(),
                             process_sp->GetAddressByteSize());
-      offset_t offset = 0;
+      lldb::offset_t offset = 0;
       uint64_t retval = regdata.GetMaxU64(&offset, 8);
       uint32_t retval_lower32 = static_cast<uint32_t>(retval & 0xffffffff);
       value.SetUInt32(retval_lower32);
@@ -463,7 +463,7 @@ bool RegisterContextDarwin_arm64::ReadRe
     if (process_sp.get()) {
       DataExtractor regdata(&fpu.v[reg - fpu_s0], 4, process_sp->GetByteOrder(),
                             process_sp->GetAddressByteSize());
-      offset_t offset = 0;
+      lldb::offset_t offset = 0;
       value.SetFloat(regdata.GetFloat(&offset));
     }
   } break;
@@ -504,7 +504,7 @@ bool RegisterContextDarwin_arm64::ReadRe
     if (process_sp.get()) {
       DataExtractor regdata(&fpu.v[reg - fpu_s0], 8, process_sp->GetByteOrder(),
                             process_sp->GetAddressByteSize());
-      offset_t offset = 0;
+      lldb::offset_t offset = 0;
       value.SetDouble(regdata.GetDouble(&offset));
     }
   } break;
