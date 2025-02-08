/*
    Determine if this a PDB image file.
  */
  (void) ResetMagickMemory(&pdb_info,0,sizeof(pdb_info));
  count=ReadBlob(image,sizeof(pdb_info.name),(unsigned char *) pdb_info.name);
  if (count != sizeof(pdb_info.name))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");