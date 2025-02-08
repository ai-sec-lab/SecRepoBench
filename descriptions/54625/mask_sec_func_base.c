int
decode_preR13_entities (BITCODE_RL start, BITCODE_RL end,
                        unsigned num_entities, BITCODE_RL size,
                        BITCODE_RL blocks_max, Bit_Chain *restrict dat,
                        Dwg_Data *restrict dwg)
{
  int error = 0;
  BITCODE_BL num = dwg->num_objects;
  unsigned long oldpos = dat->byte;

  dat->bit = 0;
  LOG_TRACE ("\n%sentities: (" FORMAT_RLx "-" FORMAT_RLx " (%u), size " FORMAT_RL ", 0x%x)\n",
             blocks_max != (BITCODE_RL)0 ? "block " : "", start, end, num_entities,
             size, blocks_max);
  LOG_INFO ("==========================================\n");
  // <MASK>
}