$NetBSD$

--- include/lldb/Host/common/NativeProcessProtocol.h.orig	2020-05-07 15:42:29.126130166 +0000
+++ include/lldb/Host/common/NativeProcessProtocol.h
@@ -402,8 +402,8 @@ protected:
 
   // interface for software breakpoints
 
-  Status SetSoftwareBreakpoint(lldb::addr_t addr, uint32_t size_hint);
-  Status RemoveSoftwareBreakpoint(lldb::addr_t addr);
+  virtual Status SetSoftwareBreakpoint(lldb::addr_t addr, uint32_t size_hint);
+  virtual Status RemoveSoftwareBreakpoint(lldb::addr_t addr);
 
   virtual llvm::Expected<llvm::ArrayRef<uint8_t>>
   GetSoftwareBreakpointTrapOpcode(size_t size_hint);
