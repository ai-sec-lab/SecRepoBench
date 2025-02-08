static int
decode_preR13_section (Dwg_Section_Type_r11 id, Bit_Chain *restrict dat,
                       Dwg_Data *restrict dwg)
{
  Dwg_Section *tbl = &dwg->header.section[id];
  Bit_Chain *hdl_dat = dat;
  int i;
  BITCODE_BL vcount;
  int error = 0;
  long unsigned int num = dwg->num_objects;
  long unsigned int pos = tbl->address;
  BITCODE_RC flag;
  BITCODE_TF name;

  LOG_TRACE ("contents table %-8s [%2d]: size:%-4u num:%-3ld (0x%lx-0x%lx)\n",
             tbl->name, id, tbl->size, (long)tbl->number, (unsigned long)tbl->address,
             (unsigned long)(tbl->address + ((unsigned long long)tbl->number * tbl->size)))
  dat->byte = tbl->address;
  dat->bit = 0;
  if ((unsigned long)(tbl->number * tbl->size) > dat->size - dat->byte)
    {
      LOG_ERROR ("Overlarge table num_entries %ld or size %ld for %-8s [%2d]",
                 (long)tbl->number, (long)tbl->size, tbl->name, id);
      return DWG_ERR_INVALIDDWG;
    }
  tbl->objid_r11 = num;
  if (dwg->num_alloced_objects < dwg->num_objects + tbl->number)
    {
      dwg->num_alloced_objects = dwg->num_objects + tbl->number;
      dwg->object = (Dwg_Object*)realloc (dwg->object,
          dwg->num_alloced_objects * sizeof (Dwg_Object));
      dwg->dirty_refs = 1;
    }

  // TODO: use the dwg.spec instead
  // MAYBE: move to a spec dwg_r11.spec, and dwg_decode_r11_NAME
#define PREP_TABLE(token)                                                     \
  // <MASK>

#define CHK_ENDPOS                                                            \
  SINCE (R_11) {                                                              \
    BITCODE_RS crc16 = bit_read_RS (dat);                                     \
    LOG_TRACE ("crc16: %04X\n", crc16);                                       \
  }                                                                           \
  pos = tbl->address + (long)((i + 1) * tbl->size);                           \
  if (pos != dat->byte)                                                       \
    {                                                                         \
      LOG_ERROR ("offset %ld", pos - dat->byte);                              \
      /*return DWG_ERR_SECTIONNOTFOUND;*/                                     \
    }