bool BitStuffer2::Decode(const Byte** ppByte, size_t& nBytesRemaining, vector<unsigned int>& dataVec, int lerc2Version) const
{
  if (!ppByte || nBytesRemaining < 1)
    return false;

  Byte numBitsByte = **ppByte;
  (*ppByte)++;
  nBytesRemaining--;

  int bits67 = numBitsByte >> 6;
  int nb = (bits67 == 0) ? 4 : 3 - bits67;

  bool doLut = (numBitsByte & (1 << 5)) ? true : false;    // bit 5
  numBitsByte &= 31;    // bits 0-4;
  int numBits = numBitsByte;

  unsigned int numElements = 0;
  if (!DecodeUInt(ppByte, nBytesRemaining, numElements, nb))
    return false;

  if (!doLut)
  {
    if (numBits > 0)    // numBits can be 0
    {
      if (lerc2Version >= 3)
      {
        if (!BitUnStuff(ppByte, nBytesRemaining, dataVec, numElements, numBits))
          return false;
      }
      else
      {
        if (!BitUnStuff_Before_Lerc2v3(ppByte, nBytesRemaining, dataVec, numElements, numBits))
          return false;
      }
    }
  }
  else
  {
    // <MASK>
  }

  return true;
}