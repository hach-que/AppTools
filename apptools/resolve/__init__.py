# vim: set ts=4 sw=4 tw=0 et ai:

__all__ = ["LibraryResolver", "ELFHasher"]

class ELFHasher():
    """Determines a hash string representing a hash section.

    Creates a safe ASCII string based on the data contents
    of the passed section.
    """
    def _add_hash_for_section(self, sha512, section):
        data = section.data()
        for i in data:
            sha512.update(hex(ord(i))[2:].zfill(2))

    """Determines a hash string representing the dynamic symbol table."""
    def _add_hash_for_dynsym(self, sha512, section):
        if (section.name != ".dynsym"):
            raise RuntimeError("_get_hash_string_for_dynsym must have dynamic symbol table.")
        for i in dyn.iter_symbols():
            sha512.update(str(i.name))
            sha512.update(str(i.entry))

    """Calculates a hash of an ELF file.

    Calculates the hash of the specified ELF file, taking
    into account the internal GNU and SYSV hashes (if available)
    and by hashing the dynamic symbols table.
    """
    def get_hash(self, elf):
        import hashlib
        sha512 = hashlib.sha512()
        section_hash = elf.get_section_by_name(".hash")
        section_gnu_hash = elf.get_section_by_name(".gnu.hash")
        section_dynsym = elf.get_section_by_name(".dynsym")
        if (section_hash != None):
            self._add_hash_for_section(sha512, section_hash)
        if (section_gnu_hash != None):
            self._add_hash_for_section(sha512, section_gnu_hash)
        if (section_dynsym != None):
            self._add_hash_for_section(sha512, section_dynsym)
        return sha512.digest()

    """Calculates a hash of an ELF file and returns a hexadecimal representation."""
    def get_readable_hash(self, elf):
        hsh = self.get_hash(elf)
        result = ""
        for i in hsh:
            result += hex(ord(i))[2:].zfill(2)
        return result

class LibraryResolver():
    def __init__(self, resolve_method, store_method):
        self._resolve_method = resolve_method
        self._store_method = store_method

    """Resolves a dynamic dependency quickly."""
    def fast_resolve(self, binary, library):
        try:
            if (not library.startswith("/")):
                return self._handle_fast_resolve(binary, library)
            else:
                return self._handle_fast_store(binary, library)
        except Exception, e:
            import traceback
            traceback.print_exc()
            pass
        return None

    def _handle_fast_resolve(self, binary, library):
        import os
        import sys
        from elftools.elf.elffile import ELFFile
        from apptools.dbus import Client
        cl = Client()
        with file(binary) as f:
            elf = ELFFile(f)
            hasher = ELFHasher()
            binary_hash = hasher.get_readable_hash(elf)

            # Contact the appd service to get the resolved filename
            # if available.
            success, result = self._resolve_method(binary_hash, library)
            if (success):
                return str(result)
            else:
                return None

    def _handle_fast_store(self, binary, library_path):
        import os
        import sys
        from elftools.elf.elffile import ELFFile
        from apptools.dbus import Client
        cl = Client()
        if (not os.path.exists(library_path)):
            return None
        with file(binary) as f:
            elf = ELFFile(f)
            hasher = ELFHasher()
            binary_hash = hasher.get_readable_hash(elf)
        with file(library_path) as f:
            elf = ELFFile(f)
            hasher = ELFHasher()
            library_hash = hasher.get_readable_hash(elf)

        # Contact the appd service to store the resolved filename.
        self._store_method(binary_hash, library_hash, library_path)
        return None
