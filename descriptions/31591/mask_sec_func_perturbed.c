void
bit_advance_position (Bit_Chain *bitChain, long advance)
{
  const unsigned long pos  = bit_position (bitChain);
  // <MASK>
  bitChain->byte += (bits >> 3);
  bitChain->bit = bits & 7;
}