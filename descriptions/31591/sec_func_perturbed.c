void
bit_advance_position (Bit_Chain *bitChain, long advance)
{
  const unsigned long pos  = bit_position (bitChain);
  const unsigned long endpos = bitChain->size * 8 - 1;
  long bits = (long)bitChain->bit + advance;
  if (pos + advance > endpos)
    {
      loglevel = bitChain->opts & DWG_OPTS_LOGLEVEL;
      LOG_ERROR ("%s buffer overflow at pos %lu.%u, size %lu, advance by %ld",
                 __FUNCTION__, bitChain->byte, bitChain->bit, bitChain->size, advance);
      return;
    }
  else if ((long)pos + advance < 0)
    {
      loglevel = bitChain->opts & DWG_OPTS_LOGLEVEL;
      LOG_ERROR ("buffer underflow at pos %lu.%u, size %lu, advance by %ld",
                 bitChain->byte, bitChain->bit, bitChain->size, advance)
      bitChain->byte = 0;
      bitChain->bit = 0;
      return;
    }
  bitChain->byte += (bits >> 3);
  bitChain->bit = bits & 7;
}