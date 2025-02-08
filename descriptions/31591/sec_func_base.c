void
bit_advance_position (Bit_Chain *dat, long advance)
{
  const unsigned long pos  = bit_position (dat);
  const unsigned long endpos = dat->size * 8 - 1;
  long bits = (long)dat->bit + advance;
  if (pos + advance > endpos)
    {
      loglevel = dat->opts & DWG_OPTS_LOGLEVEL;
      LOG_ERROR ("%s buffer overflow at pos %lu.%u, size %lu, advance by %ld",
                 __FUNCTION__, dat->byte, dat->bit, dat->size, advance);
      return;
    }
  else if ((long)pos + advance < 0)
    {
      loglevel = dat->opts & DWG_OPTS_LOGLEVEL;
      LOG_ERROR ("buffer underflow at pos %lu.%u, size %lu, advance by %ld",
                 dat->byte, dat->bit, dat->size, advance)
      dat->byte = 0;
      dat->bit = 0;
      return;
    }
  dat->byte += (bits >> 3);
  dat->bit = bits & 7;
}