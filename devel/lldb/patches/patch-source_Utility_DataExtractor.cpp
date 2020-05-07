$NetBSD$

--- source/Utility/DataExtractor.cpp.orig	2019-05-23 05:12:11.000000000 +0000
+++ source/Utility/DataExtractor.cpp
@@ -40,21 +40,22 @@
 using namespace lldb;
 using namespace lldb_private;
 
-static inline uint16_t ReadInt16(const unsigned char *ptr, offset_t offset) {
+static inline uint16_t ReadInt16(const unsigned char *ptr,
+                                 lldb::offset_t offset) {
   uint16_t value;
   memcpy(&value, ptr + offset, 2);
   return value;
 }
 
 static inline uint32_t ReadInt32(const unsigned char *ptr,
-                                 offset_t offset = 0) {
+                                 lldb::offset_t offset = 0) {
   uint32_t value;
   memcpy(&value, ptr + offset, 4);
   return value;
 }
 
 static inline uint64_t ReadInt64(const unsigned char *ptr,
-                                 offset_t offset = 0) {
+                                 lldb::offset_t offset = 0) {
   uint64_t value;
   memcpy(&value, ptr + offset, 8);
   return value;
@@ -67,21 +68,21 @@ static inline uint16_t ReadInt16(const v
 }
 
 static inline uint16_t ReadSwapInt16(const unsigned char *ptr,
-                                     offset_t offset) {
+                                     lldb::offset_t offset) {
   uint16_t value;
   memcpy(&value, ptr + offset, 2);
   return llvm::ByteSwap_16(value);
 }
 
 static inline uint32_t ReadSwapInt32(const unsigned char *ptr,
-                                     offset_t offset) {
+                                     lldb::offset_t offset) {
   uint32_t value;
   memcpy(&value, ptr + offset, 4);
   return llvm::ByteSwap_32(value);
 }
 
 static inline uint64_t ReadSwapInt64(const unsigned char *ptr,
-                                     offset_t offset) {
+                                     lldb::offset_t offset) {
   uint64_t value;
   memcpy(&value, ptr + offset, 8);
   return llvm::ByteSwap_64(value);
@@ -126,7 +127,7 @@ DataExtractor::DataExtractor()
 
 // This constructor allows us to use data that is owned by someone else. The
 // data must stay around as long as this object is valid.
-DataExtractor::DataExtractor(const void *data, offset_t length,
+DataExtractor::DataExtractor(const void *data, lldb::offset_t length,
                              ByteOrder endian, uint32_t addr_size,
                              uint32_t target_byte_size /*=1*/)
     : m_start(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(data))),
@@ -156,14 +157,14 @@ DataExtractor::DataExtractor(const DataB
 // the shared data will stay around as long as any object contains a reference
 // to that data. The endian swap and address size settings are copied from
 // "data".
-DataExtractor::DataExtractor(const DataExtractor &data, offset_t offset,
-                             offset_t length, uint32_t target_byte_size /*=1*/)
+DataExtractor::DataExtractor(const DataExtractor &data, lldb::offset_t offset,
+                             lldb::offset_t length, uint32_t target_byte_size /*=1*/)
     : m_start(nullptr), m_end(nullptr), m_byte_order(data.m_byte_order),
       m_addr_size(data.m_addr_size), m_data_sp(),
       m_target_byte_size(target_byte_size) {
   assert(m_addr_size == 4 || m_addr_size == 8);
   if (data.ValidOffset(offset)) {
-    offset_t bytes_available = data.GetByteSize() - offset;
+    lldb::offset_t bytes_available = data.GetByteSize() - offset;
     if (length > bytes_available)
       length = bytes_available;
     SetData(data, offset, length);
@@ -224,7 +225,7 @@ size_t DataExtractor::GetSharedDataOffse
 // and can extract from it. If this object refers to any shared data upon
 // entry, the reference to that data will be released. Is SWAP is set to true,
 // any data extracted will be endian swapped.
-lldb::offset_t DataExtractor::SetData(const void *bytes, offset_t length,
+lldb::offset_t DataExtractor::SetData(const void *bytes, lldb::offset_t length,
                                       ByteOrder endian) {
   m_byte_order = endian;
   m_data_sp.reset();
@@ -249,8 +250,8 @@ lldb::offset_t DataExtractor::SetData(co
 // exist at least as long as this object refers to those bytes. The address
 // size and endian swap settings are copied from the current values in "data".
 lldb::offset_t DataExtractor::SetData(const DataExtractor &data,
-                                      offset_t data_offset,
-                                      offset_t data_length) {
+                                      lldb::offset_t data_offset,
+                                      lldb::offset_t data_length) {
   m_addr_size = data.m_addr_size;
   assert(m_addr_size == 4 || m_addr_size == 8);
   // If "data" contains shared pointer to data, then we can use that
@@ -282,8 +283,8 @@ lldb::offset_t DataExtractor::SetData(co
 // needed. The address size and endian swap settings will remain unchanged from
 // their current settings.
 lldb::offset_t DataExtractor::SetData(const DataBufferSP &data_sp,
-                                      offset_t data_offset,
-                                      offset_t data_length) {
+                                      lldb::offset_t data_offset,
+                                      lldb::offset_t data_length) {
   m_start = m_end = nullptr;
 
   if (data_length > 0) {
@@ -317,7 +318,7 @@ lldb::offset_t DataExtractor::SetData(co
 // pointed to by "offset_ptr".
 //
 // RETURNS the byte that was extracted, or zero on failure.
-uint8_t DataExtractor::GetU8(offset_t *offset_ptr) const {
+uint8_t DataExtractor::GetU8(lldb::offset_t *offset_ptr) const {
   const uint8_t *data = static_cast<const uint8_t *>(GetData(offset_ptr, 1));
   if (data)
     return *data;
@@ -330,7 +331,7 @@ uint8_t DataExtractor::GetU8(offset_t *o
 // RETURNS the non-nullptr buffer pointer upon successful extraction of
 // all the requested bytes, or nullptr when the data is not available in the
 // buffer due to being out of bounds, or insufficient data.
-void *DataExtractor::GetU8(offset_t *offset_ptr, void *dst,
+void *DataExtractor::GetU8(lldb::offset_t *offset_ptr, void *dst,
                            uint32_t count) const {
   const uint8_t *data =
       static_cast<const uint8_t *>(GetData(offset_ptr, count));
@@ -348,7 +349,7 @@ void *DataExtractor::GetU8(offset_t *off
 // "offset_ptr".
 //
 // RETURNS the uint16_t that was extracted, or zero on failure.
-uint16_t DataExtractor::GetU16(offset_t *offset_ptr) const {
+uint16_t DataExtractor::GetU16(lldb::offset_t *offset_ptr) const {
   uint16_t val = 0;
   const uint8_t *data =
       static_cast<const uint8_t *>(GetData(offset_ptr, sizeof(val)));
@@ -361,7 +362,7 @@ uint16_t DataExtractor::GetU16(offset_t
   return val;
 }
 
-uint16_t DataExtractor::GetU16_unchecked(offset_t *offset_ptr) const {
+uint16_t DataExtractor::GetU16_unchecked(lldb::offset_t *offset_ptr) const {
   uint16_t val;
   if (m_byte_order == endian::InlHostByteOrder())
     val = ReadInt16(m_start, *offset_ptr);
@@ -371,7 +372,7 @@ uint16_t DataExtractor::GetU16_unchecked
   return val;
 }
 
-uint32_t DataExtractor::GetU32_unchecked(offset_t *offset_ptr) const {
+uint32_t DataExtractor::GetU32_unchecked(lldb::offset_t *offset_ptr) const {
   uint32_t val;
   if (m_byte_order == endian::InlHostByteOrder())
     val = ReadInt32(m_start, *offset_ptr);
@@ -381,7 +382,7 @@ uint32_t DataExtractor::GetU32_unchecked
   return val;
 }
 
-uint64_t DataExtractor::GetU64_unchecked(offset_t *offset_ptr) const {
+uint64_t DataExtractor::GetU64_unchecked(lldb::offset_t *offset_ptr) const {
   uint64_t val;
   if (m_byte_order == endian::InlHostByteOrder())
     val = ReadInt64(m_start, *offset_ptr);
@@ -397,7 +398,7 @@ uint64_t DataExtractor::GetU64_unchecked
 // RETURNS the non-nullptr buffer pointer upon successful extraction of
 // all the requested bytes, or nullptr when the data is not available in the
 // buffer due to being out of bounds, or insufficient data.
-void *DataExtractor::GetU16(offset_t *offset_ptr, void *void_dst,
+void *DataExtractor::GetU16(lldb::offset_t *offset_ptr, void *void_dst,
                             uint32_t count) const {
   const size_t src_size = sizeof(uint16_t) * count;
   const uint16_t *src =
@@ -426,7 +427,7 @@ void *DataExtractor::GetU16(offset_t *of
 // "offset_ptr".
 //
 // RETURNS the uint32_t that was extracted, or zero on failure.
-uint32_t DataExtractor::GetU32(offset_t *offset_ptr) const {
+uint32_t DataExtractor::GetU32(lldb::offset_t *offset_ptr) const {
   uint32_t val = 0;
   const uint8_t *data =
       static_cast<const uint8_t *>(GetData(offset_ptr, sizeof(val)));
@@ -446,7 +447,7 @@ uint32_t DataExtractor::GetU32(offset_t
 // RETURNS the non-nullptr buffer pointer upon successful extraction of
 // all the requested bytes, or nullptr when the data is not available in the
 // buffer due to being out of bounds, or insufficient data.
-void *DataExtractor::GetU32(offset_t *offset_ptr, void *void_dst,
+void *DataExtractor::GetU32(lldb::offset_t *offset_ptr, void *void_dst,
                             uint32_t count) const {
   const size_t src_size = sizeof(uint32_t) * count;
   const uint32_t *src =
@@ -475,7 +476,7 @@ void *DataExtractor::GetU32(offset_t *of
 // "offset_ptr".
 //
 // RETURNS the uint64_t that was extracted, or zero on failure.
-uint64_t DataExtractor::GetU64(offset_t *offset_ptr) const {
+uint64_t DataExtractor::GetU64(lldb::offset_t *offset_ptr) const {
   uint64_t val = 0;
   const uint8_t *data =
       static_cast<const uint8_t *>(GetData(offset_ptr, sizeof(val)));
@@ -494,7 +495,7 @@ uint64_t DataExtractor::GetU64(offset_t
 // Get multiple consecutive 64 bit values. Return true if the entire read
 // succeeds and increment the offset pointed to by offset_ptr, else return
 // false and leave the offset pointed to by offset_ptr unchanged.
-void *DataExtractor::GetU64(offset_t *offset_ptr, void *void_dst,
+void *DataExtractor::GetU64(lldb::offset_t *offset_ptr, void *void_dst,
                             uint32_t count) const {
   const size_t src_size = sizeof(uint64_t) * count;
   const uint64_t *src =
@@ -519,13 +520,13 @@ void *DataExtractor::GetU64(offset_t *of
   return nullptr;
 }
 
-uint32_t DataExtractor::GetMaxU32(offset_t *offset_ptr,
+uint32_t DataExtractor::GetMaxU32(lldb::offset_t *offset_ptr,
                                   size_t byte_size) const {
   lldbassert(byte_size > 0 && byte_size <= 4 && "GetMaxU32 invalid byte_size!");
   return GetMaxU64(offset_ptr, byte_size);
 }
 
-uint64_t DataExtractor::GetMaxU64(offset_t *offset_ptr,
+uint64_t DataExtractor::GetMaxU64(lldb::offset_t *offset_ptr,
                                   size_t byte_size) const {
   lldbassert(byte_size > 0 && byte_size <= 8 && "GetMaxU64 invalid byte_size!");
   switch (byte_size) {
@@ -549,7 +550,7 @@ uint64_t DataExtractor::GetMaxU64(offset
   return 0;
 }
 
-uint64_t DataExtractor::GetMaxU64_unchecked(offset_t *offset_ptr,
+uint64_t DataExtractor::GetMaxU64_unchecked(lldb::offset_t *offset_ptr,
                                             size_t byte_size) const {
   switch (byte_size) {
   case 1:
@@ -569,12 +570,12 @@ uint64_t DataExtractor::GetMaxU64_unchec
   return 0;
 }
 
-int64_t DataExtractor::GetMaxS64(offset_t *offset_ptr, size_t byte_size) const {
+int64_t DataExtractor::GetMaxS64(lldb::offset_t *offset_ptr, size_t byte_size) const {
   uint64_t u64 = GetMaxU64(offset_ptr, byte_size);
   return llvm::SignExtend64(u64, 8 * byte_size);
 }
 
-uint64_t DataExtractor::GetMaxU64Bitfield(offset_t *offset_ptr, size_t size,
+uint64_t DataExtractor::GetMaxU64Bitfield(lldb::offset_t *offset_ptr, size_t size,
                                           uint32_t bitfield_bit_size,
                                           uint32_t bitfield_bit_offset) const {
   uint64_t uval64 = GetMaxU64(offset_ptr, size);
@@ -592,7 +593,7 @@ uint64_t DataExtractor::GetMaxU64Bitfiel
   return uval64;
 }
 
-int64_t DataExtractor::GetMaxS64Bitfield(offset_t *offset_ptr, size_t size,
+int64_t DataExtractor::GetMaxS64Bitfield(lldb::offset_t *offset_ptr, size_t size,
                                          uint32_t bitfield_bit_size,
                                          uint32_t bitfield_bit_offset) const {
   int64_t sval64 = GetMaxS64(offset_ptr, size);
@@ -612,7 +613,7 @@ int64_t DataExtractor::GetMaxS64Bitfield
   return sval64;
 }
 
-float DataExtractor::GetFloat(offset_t *offset_ptr) const {
+float DataExtractor::GetFloat(lldb::offset_t *offset_ptr) const {
   typedef float float_type;
   float_type val = 0.0;
   const size_t src_size = sizeof(float_type);
@@ -631,7 +632,7 @@ float DataExtractor::GetFloat(offset_t *
   return val;
 }
 
-double DataExtractor::GetDouble(offset_t *offset_ptr) const {
+double DataExtractor::GetDouble(lldb::offset_t *offset_ptr) const {
   typedef double float_type;
   float_type val = 0.0;
   const size_t src_size = sizeof(float_type);
@@ -650,7 +651,7 @@ double DataExtractor::GetDouble(offset_t
   return val;
 }
 
-long double DataExtractor::GetLongDouble(offset_t *offset_ptr) const {
+long double DataExtractor::GetLongDouble(lldb::offset_t *offset_ptr) const {
   long double val = 0.0;
 #if defined(__i386__) || defined(__amd64__) || defined(__x86_64__) ||          \
     defined(_M_IX86) || defined(_M_IA64) || defined(_M_X64)
@@ -669,12 +670,12 @@ long double DataExtractor::GetLongDouble
 // extracting any address values.
 //
 // RETURNS the address that was extracted, or zero on failure.
-uint64_t DataExtractor::GetAddress(offset_t *offset_ptr) const {
+uint64_t DataExtractor::GetAddress(lldb::offset_t *offset_ptr) const {
   assert(m_addr_size == 4 || m_addr_size == 8);
   return GetMaxU64(offset_ptr, m_addr_size);
 }
 
-uint64_t DataExtractor::GetAddress_unchecked(offset_t *offset_ptr) const {
+uint64_t DataExtractor::GetAddress_unchecked(lldb::offset_t *offset_ptr) const {
   assert(m_addr_size == 4 || m_addr_size == 8);
   return GetMaxU64_unchecked(offset_ptr, m_addr_size);
 }
@@ -685,12 +686,12 @@ uint64_t DataExtractor::GetAddress_unche
 // extracting any pointer values.
 //
 // RETURNS the pointer that was extracted, or zero on failure.
-uint64_t DataExtractor::GetPointer(offset_t *offset_ptr) const {
+uint64_t DataExtractor::GetPointer(lldb::offset_t *offset_ptr) const {
   assert(m_addr_size == 4 || m_addr_size == 8);
   return GetMaxU64(offset_ptr, m_addr_size);
 }
 
-size_t DataExtractor::ExtractBytes(offset_t offset, offset_t length,
+size_t DataExtractor::ExtractBytes(lldb::offset_t offset, lldb::offset_t length,
                                    ByteOrder dst_byte_order, void *dst) const {
   const uint8_t *src = PeekData(offset, length);
   if (src) {
@@ -709,8 +710,8 @@ size_t DataExtractor::ExtractBytes(offse
 }
 
 // Extract data as it exists in target memory
-lldb::offset_t DataExtractor::CopyData(offset_t offset, offset_t length,
-                                       void *dst) const {
+lldb::offset_t DataExtractor::CopyData(lldb::offset_t offset,
+                                       lldb::offset_t length, void *dst) const {
   const uint8_t *src = PeekData(offset, length);
   if (src) {
     ::memcpy(dst, src, length);
@@ -721,8 +722,9 @@ lldb::offset_t DataExtractor::CopyData(o
 
 // Extract data and swap if needed when doing the copy
 lldb::offset_t
-DataExtractor::CopyByteOrderedData(offset_t src_offset, offset_t src_len,
-                                   void *dst_void_ptr, offset_t dst_len,
+DataExtractor::CopyByteOrderedData(lldb::offset_t src_offset,
+                                   lldb::offset_t src_len,
+                                   void *dst_void_ptr, lldb::offset_t dst_len,
                                    ByteOrder dst_byte_order) const {
   // Validate the source info
   if (!ValidOffsetForDataOfSize(src_offset, src_len))
@@ -815,7 +817,7 @@ DataExtractor::CopyByteOrderedData(offse
 // If the offset pointed to by "offset_ptr" is out of bounds, or if "length" is
 // non-zero and there aren't enough available bytes, nullptr will be returned
 // and "offset_ptr" will not be updated.
-const char *DataExtractor::GetCStr(offset_t *offset_ptr) const {
+const char *DataExtractor::GetCStr(lldb::offset_t *offset_ptr) const {
   const char *cstr = reinterpret_cast<const char *>(PeekData(*offset_ptr, 1));
   if (cstr) {
     const char *cstr_end = cstr;
@@ -846,7 +848,8 @@ const char *DataExtractor::GetCStr(offse
 // plus the length of the field is out of bounds, or if the field does not
 // contain a NULL terminator byte, nullptr will be returned and "offset_ptr"
 // will not be updated.
-const char *DataExtractor::GetCStr(offset_t *offset_ptr, offset_t len) const {
+const char *DataExtractor::GetCStr(lldb::offset_t *offset_ptr,
+                                   lldb::offset_t len) const {
   const char *cstr = reinterpret_cast<const char *>(PeekData(*offset_ptr, len));
   if (cstr != nullptr) {
     if (memchr(cstr, '\0', len) == nullptr) {
@@ -864,7 +867,7 @@ const char *DataExtractor::GetCStr(offse
 //
 // Returns a valid C string pointer if "offset" is a valid offset in this
 // object's data, else nullptr is returned.
-const char *DataExtractor::PeekCStr(offset_t offset) const {
+const char *DataExtractor::PeekCStr(lldb::offset_t offset) const {
   return reinterpret_cast<const char *>(PeekData(offset, 1));
 }
 
@@ -874,7 +877,7 @@ const char *DataExtractor::PeekCStr(offs
 // byte.
 //
 // Returned the extracted integer value.
-uint64_t DataExtractor::GetULEB128(offset_t *offset_ptr) const {
+uint64_t DataExtractor::GetULEB128(lldb::offset_t *offset_ptr) const {
   const uint8_t *src = PeekData(*offset_ptr, 1);
   if (src == nullptr)
     return 0;
@@ -907,7 +910,7 @@ uint64_t DataExtractor::GetULEB128(offse
 // byte.
 //
 // Returned the extracted integer value.
-int64_t DataExtractor::GetSLEB128(offset_t *offset_ptr) const {
+int64_t DataExtractor::GetSLEB128(lldb::offset_t *offset_ptr) const {
   const uint8_t *src = PeekData(*offset_ptr, 1);
   if (src == nullptr)
     return 0;
@@ -947,7 +950,7 @@ int64_t DataExtractor::GetSLEB128(offset
 // extracted byte.
 //
 // Returns the number of bytes consumed during the extraction.
-uint32_t DataExtractor::Skip_LEB128(offset_t *offset_ptr) const {
+uint32_t DataExtractor::Skip_LEB128(lldb::offset_t *offset_ptr) const {
   uint32_t bytes_consumed = 0;
   const uint8_t *src = PeekData(*offset_ptr, 1);
   if (src == nullptr)
@@ -973,16 +976,17 @@ uint32_t DataExtractor::Skip_LEB128(offs
 // printf style formatting string. If "type_format" is nullptr, then an
 // appropriate format string will be used for the supplied "type". If the
 // stream "s" is nullptr, then the output will be send to Log().
-lldb::offset_t DataExtractor::PutToLog(Log *log, offset_t start_offset,
-                                       offset_t length, uint64_t base_addr,
+lldb::offset_t DataExtractor::PutToLog(Log *log, lldb::offset_t start_offset,
+                                       lldb::offset_t length,
+                                       uint64_t base_addr,
                                        uint32_t num_per_line,
                                        DataExtractor::Type type,
                                        const char *format) const {
   if (log == nullptr)
     return start_offset;
 
-  offset_t offset;
-  offset_t end_offset;
+  lldb::offset_t offset;
+  lldb::offset_t end_offset;
   uint32_t count;
   StreamString sstr;
   for (offset = start_offset, end_offset = offset + length, count = 0;
@@ -1074,7 +1078,7 @@ bool DataExtractor::Append(DataExtractor
   return true;
 }
 
-bool DataExtractor::Append(void *buf, offset_t length) {
+bool DataExtractor::Append(void *buf, lldb::offset_t length) {
   if (buf == nullptr)
     return false;
 
