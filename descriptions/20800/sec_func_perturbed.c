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

    if (!struct_fits_in_pe(pe, codeview_header, CV_HEADER))
      continue;

    if (yr_le32toh(codeview_header->dwSignature) == CVINFO_PDB20_CVSIGNATURE)
    {
      PCV_INFO_PDB20 pdb20 = (PCV_INFO_PDB20) codeview_header;
      
      if (struct_fits_in_pe(pe, pdb20, CV_INFO_PDB20))
        pdb_path = (char*) (pdb20->PdbFileName);
    }
    else if (yr_le32toh(codeview_header->dwSignature) == CVINFO_PDB70_CVSIGNATURE)
    {
      PCV_INFO_PDB70 pdb70 = (PCV_INFO_PDB70) codeview_header;
      
      if (struct_fits_in_pe(pe, pdb70, CV_INFO_PDB70))
        pdb_path = (char*) (pdb70->PdbFileName);
    }

    if (pdb_path != NULL)
    {
      pdb_path_len = strnlen(
          pdb_path, yr_min(available_space(pe, pdb_path), MAX_PATH));

      if (pdb_path_len > 0 && pdb_path_len < MAX_PATH)
      {
        set_sized_string(pdb_path, pdb_path_len, pe->object, "pdb_path");
        break;
      }
    }
  }
  
  return;
}