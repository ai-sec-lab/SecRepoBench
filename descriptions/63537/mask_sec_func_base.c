static int
dxf_objects_read (Bit_Chain *restrict dat, Dwg_Data *restrict dwg)
{
  char name[80];
  Dxf_Pair *pair = dxf_read_pair (dat);
  while (pair != NULL)
    {
      while (pair != NULL && pair->code == 0 && pair->value.s)
        {
          // <MASK>
        }
      dxf_free_pair (pair);
      pair = dxf_read_pair (dat);
      DXF_CHECK_EOF;
    }
  dxf_free_pair (pair);
  return 0;
}