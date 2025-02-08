IMPORT_EXPORT_FUNCTION* pe_parse_exports(
    PE* pe)
{
  IMPORT_EXPORT_FUNCTION* head = NULL;
  IMPORT_EXPORT_FUNCTION* tail = NULL;

  PIMAGE_DATA_DIRECTORY directory;
  PIMAGE_EXPORT_DIRECTORY exports;

  DWORD* names;
  WORD* ordinals;

  int64_t offset;
  uint32_t index;
  size_t remaining;

  int num_exports = 0;

  // If not a PE file, return UNDEFINED

  if (pe == NULL)
    return NULL;

  directory = pe_get_directory_entry(
      pe, IMAGE_DIRECTORY_ENTRY_EXPORT);

  if (yr_le32toh(directory->VirtualAddress) == 0)
    return NULL;

  offset = pe_rva_to_offset(pe, yr_le32toh(directory->VirtualAddress));

  if (offset < 0)
    return NULL;

  exports = (PIMAGE_EXPORT_DIRECTORY) (pe->data + offset);

  if (!struct_fits_in_pe(pe, exports, IMAGE_EXPORT_DIRECTORY))
    return NULL;

  offset = pe_rva_to_offset(pe, yr_le32toh(exports->AddressOfNames));

  if (offset < 0)
    return NULL;

  if (yr_le32toh(exports->NumberOfFunctions) > MAX_PE_EXPORTS ||
      yr_le32toh(exports->NumberOfFunctions) * sizeof(DWORD) > pe->data_size - offset)
    return NULL;

  names = (DWORD*)(pe->data + offset);

  offset = pe_rva_to_offset(pe, yr_le32toh(exports->AddressOfNameOrdinals));

  if (offset < 0)
    return NULL;

  ordinals = (WORD*)(pe->data + offset);

  // Walk the number of functions, not the number of names as each exported
  // symbol has an ordinal value, but names are optional.

  for (index = 0; index < yr_le32toh(exports->NumberOfFunctions); index++)
  {
    IMPORT_EXPORT_FUNCTION* exported_func;
    uint16_t ordinal = 0;
    char* name;

    // <MASK>
  }

  set_integer(num_exports, pe->object, "number_of_exports");
  return head;
}