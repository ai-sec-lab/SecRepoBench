/*
    Determine if this a PDB image file.
  */
  count=ReadBlob(image,sizeof(pdb_info.name),(unsigned char *) pdb_info.name);
  if (count != sizeof(pdb_info.name))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");