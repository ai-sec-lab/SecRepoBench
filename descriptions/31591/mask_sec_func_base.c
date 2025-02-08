void
bit_advance_position (Bit_Chain *dat, long advance)
{
  const unsigned long pos  = bit_position (dat);
  // <MASK>
  dat->byte += (bits >> 3);
  dat->bit = bits & 7;
}