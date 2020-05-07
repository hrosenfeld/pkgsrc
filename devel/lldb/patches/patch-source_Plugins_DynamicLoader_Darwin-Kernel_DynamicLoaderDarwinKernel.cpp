$NetBSD$

--- source/Plugins/DynamicLoader/Darwin-Kernel/DynamicLoaderDarwinKernel.cpp.orig	2019-05-23 11:14:47.000000000 +0000
+++ source/Plugins/DynamicLoader/Darwin-Kernel/DynamicLoaderDarwinKernel.cpp
@@ -243,7 +243,7 @@ DynamicLoaderDarwinKernel::SearchForKern
       if (process->ReadMemoryFromInferior (kernel_addresses_64[i], uval, 8, read_err) == 8)
       {
           DataExtractor data (&uval, 8, process->GetByteOrder(), process->GetAddressByteSize());
-          offset_t offset = 0;
+          lldb::offset_t offset = 0;
           uint64_t addr = data.GetU64 (&offset);
           if (CheckForKernelImageAtAddress(addr, process).IsValid()) {
               return addr;
@@ -257,7 +257,7 @@ DynamicLoaderDarwinKernel::SearchForKern
       if (process->ReadMemoryFromInferior (kernel_addresses_32[i], uval, 4, read_err) == 4)
       {
           DataExtractor data (&uval, 4, process->GetByteOrder(), process->GetAddressByteSize());
-          offset_t offset = 0;
+          lldb::offset_t offset = 0;
           uint32_t addr = data.GetU32 (&offset);
           if (CheckForKernelImageAtAddress(addr, process).IsValid()) {
               return addr;
