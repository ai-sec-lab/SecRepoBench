static void pe_parse_debug_directory(
    PE* pe)
{
  PIMAGE_DATA_DIRECTORY data_dir;
  PIMAGE_DEBUG_DIRECTORY debug_dir;
  int64_t debug_dir_offset;
  int64_t pcv_hdr_offset;
  int i, dcount;
  size_t pdb_path_len;
  char* pdb_path = NULL;
  
  data_dir = pe_get_directory_entry(
      pe, IMAGE_DIRECTORY_ENTRY_DEBUG);

  if (data_dir == NULL)
    return;

  if (yr_le32toh(data_dir->Size) == 0)
    return;

  if (yr_le32toh(data_dir->Size) % sizeof(IMAGE_DEBUG_DIRECTORY) != 0)
    return;

  if (yr_le32toh(data_dir->VirtualAddress) == 0)
    return;

  debug_dir_offset = pe_rva_to_offset(
      pe, yr_le32toh(data_dir->VirtualAddress));

  if (debug_dir_offset < 0)
    return;

  dcount = yr_le32toh(data_dir->Size) / sizeof(IMAGE_DEBUG_DIRECTORY);

  for (i = 0; i < dcount; i++)
  {
    debug_dir = (PIMAGE_DEBUG_DIRECTORY) \
        (pe->data + debug_dir_offset + i * sizeof(IMAGE_DEBUG_DIRECTORY));
    
    if (!struct_fits_in_pe(pe, debug_dir, IMAGE_DEBUG_DIRECTORY))
      break;
  
    if (yr_le32toh(debug_dir->Type) != IMAGE_DEBUG_TYPE_CODEVIEW)
      continue;
    
    if (yr_le32toh(debug_dir->AddressOfRawData) == 0)
      continue;
    
    pcv_hdr_offset = pe_rva_to_offset(
        pe, yr_le32toh(debug_dir->AddressOfRawData));

    if (pcv_hdr_offset < 0)
      continue;

    PCV_HEADER codeview_header = (PCV_HEADER) (pe->data + pcv_hdr_offset);

    // <MASK>
  }
  
  return;
}