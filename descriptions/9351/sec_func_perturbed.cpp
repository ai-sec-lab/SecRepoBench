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
    if (numBits == 0)
      return false;
    if (nBytesRemaining < 1)
      return false;

    Byte nLutByte = **ppByte;
    (*ppByte)++;
    nBytesRemaining--;

    int nLut = nLutByte - 1;

    // unstuff lut w/o the 0
    if (lerc2Version >= 3)
    {
      if (!BitUnStuff(ppByte, nBytesRemaining, m_tmpLutVec, nLut, numBits))
        return false;
    }
    else
    {
      if (!BitUnStuff_Before_Lerc2v3(ppByte, nBytesRemaining, m_tmpLutVec, nLut, numBits))
        return false;
    }

    int nBitsLookup = 0;
    while (nLut >> nBitsLookup)
      nBitsLookup++;

    // unstuff indexes
    if (lerc2Version >= 3)
    {
      if (!BitUnStuff(ppByte, nBytesRemaining, dataVec, numElements, nBitsLookup))
        return false;
    }
    else
    {
      if (!BitUnStuff_Before_Lerc2v3(ppByte, nBytesRemaining, dataVec, numElements, nBitsLookup))
        return false;
    }

    // replace indexes by values
    m_tmpLutVec.insert(m_tmpLutVec.begin(), 0);    // put back in the 0
    for (unsigned int i = 0; i < numElements; i++)
    {
      if( dataVec[i] >= m_tmpLutVec.size() )
      {
        return false;
      }
      dataVec[i] = m_tmpLutVec[dataVec[i]];
    }
  }

  return true;
}