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