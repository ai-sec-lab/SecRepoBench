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

    int nBitsLut = 0;
    while (nLut >> nBitsLut)
      nBitsLut++;

    // unstuff indexes
    if (lerc2Version >= 3)
    {
      if (!BitUnStuff(ppByte, nBytesRemaining, dataVec, numElements, nBitsLut))
        return false;
    }
    else
    {
      if (!BitUnStuff_Before_Lerc2v3(ppByte, nBytesRemaining, dataVec, numElements, nBitsLut))
        return false;
    }

    // replace indexes by values
    m_tmpLutVec.insert(m_tmpLutVec.begin(), 0);    // put back in the 0
    for (unsigned int i = 0; i < numElements; i++)
      dataVec[i] = m_tmpLutVec[dataVec[i]];