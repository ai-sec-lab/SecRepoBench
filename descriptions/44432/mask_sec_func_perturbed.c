static int
read_file_header (Bit_Chain *restrict dat,
                  r2007_file_header *restrict file_header)
{
  BITCODE_RC data[0x3d8]; // 0x400 - 5 long
  BITCODE_RC *pedata;
  uint64_t seqence_crc;
  uint64_t seqence_key;
  uint64_t compr_crc;
  int32_t compr_len, len2;
  int i;
  int error = 0, errcount = 0;
  const int pedatalength = 3 * 239; // size of pedata

  dat->byte = 0x80;
  LOG_TRACE ("\n=== File header ===\n")
  memset (file_header, 0, sizeof (r2007_file_header));
  memset (data, 0, 0x3d8);
  bit_read_fixed (dat, data, 0x3d8);
  pedata = decode_rs (data, 3, 239, 0x3d8);
  if (!pedata)
    return DWG_ERR_OUTOFMEM;

  // Note: This is unportable to big-endian
  seqence_crc = *((uint64_t *)pedata);
  seqence_key = *((uint64_t *)&pedata[8]);
  compr_crc = *((uint64_t *)&pedata[16]);
  compr_len = *((int32_t *)&pedata[24]);
  len2 = *((int32_t *)&pedata[28]);
  LOG_TRACE ("seqence_crc64: %016lX\n", (unsigned long)seqence_crc);
  LOG_TRACE ("seqence_key:   %016lX\n", (unsigned long)seqence_key);
  LOG_TRACE ("compr_crc64:   %016lX\n", (unsigned long)compr_crc);
  LOG_TRACE ("compr_len:     %d\n", (int)compr_len); // only this is used
  LOG_TRACE ("len2:          %d\n", (int)len2);      // <MASK>
}