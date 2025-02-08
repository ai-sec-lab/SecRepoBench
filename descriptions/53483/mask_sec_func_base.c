static int
decode_R13_R2000 (Bit_Chain *restrict dat, Dwg_Data *restrict dwg)
{
  Dwg_Object *obj = NULL;
  unsigned int section_size = 0;
  unsigned char sgdc[2];
  unsigned int crc, crc2;
  long unsigned int size;
  long unsigned int endpos;
  long unsigned int lastmap;
  long unsigned int startpos;
  long unsigned int object_begin;
  long unsigned int object_end;
  long unsigned int pvz;
  BITCODE_BL j, k;
  int error = 0;
  const char *section_names[]
      = { "AcDb:Header", "AcDb:Classes", "AcDb:Handles",
          "2NDHEADER",   "AcDb:Template",  "AcDb:AuxHeader" };

  {
    int i;
    Dwg_Header *_obj = &dwg->header;
    Bit_Chain *hdl_dat = dat;
    dat->byte = 0x06;
    // clang-format off
    #include "header.spec"
    // clang-format on
  }

  /* Section Locator Records 0x15 */
  // <MASK>
}