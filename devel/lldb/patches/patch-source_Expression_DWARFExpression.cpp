$NetBSD$

--- source/Expression/DWARFExpression.cpp.orig	2019-06-14 13:01:16.000000000 +0000
+++ source/Expression/DWARFExpression.cpp
@@ -583,7 +583,7 @@ static bool ReadRegisterValueAsScalar(Re
   return false;
 }
 
-static offset_t GetOpcodeDataSize(const DataExtractor &data,
+static lldb::offset_t GetOpcodeDataSize(const DataExtractor &data,
                                   const lldb::offset_t data_offset,
                                   const uint8_t op) {
   lldb::offset_t offset = data_offset;
@@ -813,7 +813,7 @@ lldb::addr_t DWARFExpression::GetLocatio
       } else
         ++curr_op_addr_idx;
     } else {
-      const offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
+      const lldb::offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
       if (op_arg_size == LLDB_INVALID_OFFSET) {
         error = true;
         break;
@@ -856,7 +856,7 @@ bool DWARFExpression::Update_DW_OP_addr(
       m_data.SetData(DataBufferSP(head_data_up.release()));
       return true;
     } else {
-      const offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
+      const lldb::offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
       if (op_arg_size == LLDB_INVALID_OFFSET)
         break;
       offset += op_arg_size;
@@ -877,7 +877,7 @@ bool DWARFExpression::ContainsThreadLoca
 
     if (op == DW_OP_form_tls_address || op == DW_OP_GNU_push_tls_address)
       return true;
-    const offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
+    const lldb::offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
     if (op_arg_size == LLDB_INVALID_OFFSET)
       return false;
     else
@@ -968,7 +968,7 @@ bool DWARFExpression::LinkThreadLocalSto
     }
 
     if (!decoded_data) {
-      const offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
+      const lldb::offset_t op_arg_size = GetOpcodeDataSize(m_data, offset, op);
       if (op_arg_size == LLDB_INVALID_OFFSET)
         return false;
       else
