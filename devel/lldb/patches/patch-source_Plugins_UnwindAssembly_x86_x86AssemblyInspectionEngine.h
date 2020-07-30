$NetBSD$

--- source/Plugins/UnwindAssembly/x86/x86AssemblyInspectionEngine.h.orig	2019-06-03 22:34:12.000000000 +0000
+++ source/Plugins/UnwindAssembly/x86/x86AssemblyInspectionEngine.h
@@ -95,6 +95,7 @@ private:
   bool push_0_pattern_p();
   bool push_imm_pattern_p();
   bool push_extended_pattern_p();
+  bool push_extended_ecx_pattern_p();
   bool push_misc_reg_p();
   bool mov_rsp_rbp_pattern_p();
   bool mov_rsp_rbx_pattern_p();
@@ -105,6 +106,7 @@ private:
   bool lea_rsp_pattern_p(int &amount);
   bool lea_rbp_rsp_pattern_p(int &amount);
   bool lea_rbx_rsp_pattern_p(int &amount);
+  bool lea_esp_ecx_pattern_p(int &amount);
   bool and_rsp_pattern_p();
   bool push_reg_p(int &regno);
   bool pop_reg_p(int &regno);
@@ -174,10 +176,12 @@ private:
   uint32_t m_machine_sp_regnum;
   uint32_t m_machine_fp_regnum;
   uint32_t m_machine_alt_fp_regnum;
+  uint32_t m_machine_cx_regnum;
   uint32_t m_lldb_ip_regnum;
   uint32_t m_lldb_sp_regnum;
   uint32_t m_lldb_fp_regnum;
   uint32_t m_lldb_alt_fp_regnum;
+  uint32_t m_lldb_cx_regnum;
 
   typedef std::map<uint32_t, lldb_reg_info> MachineRegnumToNameAndLLDBRegnum;
 
