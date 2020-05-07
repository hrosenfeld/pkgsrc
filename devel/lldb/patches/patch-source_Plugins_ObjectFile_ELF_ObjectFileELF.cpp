$NetBSD$

--- source/Plugins/ObjectFile/ELF/ObjectFileELF.cpp.orig	2019-07-10 16:10:43.000000000 +0000
+++ source/Plugins/ObjectFile/ELF/ObjectFileELF.cpp
@@ -75,6 +75,8 @@ const elf_word LLDB_NT_NETBSD_IDENT_DESC
 const elf_word LLDB_NT_NETBSD_IDENT_NAMESZ = 7;
 const elf_word LLDB_NT_NETBSD_PROCINFO = 1;
 
+const elf_word LLDB_NT_SUNOS_UTSNAME = 15;
+
 // GNU ABI note OS constants
 const elf_word LLDB_NT_GNU_ABI_OS_LINUX = 0x00;
 const elf_word LLDB_NT_GNU_ABI_OS_HURD = 0x01;
@@ -1333,6 +1335,13 @@ ObjectFileELF::RefineModuleDetailsFromNo
           // cases (e.g. compile with -nostdlib) Hence set OS to Linux
           arch_spec.GetTriple().setOS(llvm::Triple::OSType::Linux);
       }
+      // Process illumos/Solaris UTSNAME note. All notes use owner "CORE".
+      else if (note.n_type == LLDB_NT_SUNOS_UTSNAME) {
+        if (strcmp(data.GetCStr(&offset), "SunOS") == 0) {
+          arch_spec.GetTriple().setOS(llvm::Triple::OSType::Solaris);
+          arch_spec.GetTriple().setVendor(llvm::Triple::VendorType::UnknownVendor);
+        }
+      }
     }
 
     // Calculate the offset of the next note just in case "offset" has been
