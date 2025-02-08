int   TABINDNode::IndexKeyCmp(GByte *pKeyValue, int nEntryNo)
{
    CPLAssert(pKeyValue);
    CPLAssert(nEntryNo >= 0 && nEntryNo < m_numEntriesInNode);

    m_poDataBlock->GotoByteInBlock(12 + nEntryNo*(m_nKeyLength+4));
    CPLAssert( m_nKeyLength <= 255 );
    GByte abyKey[255];
    if( m_poDataBlock->ReadBytes(m_nKeyLength, abyKey) != 0 )
        return -1;
    return memcmp(pKeyValue, abyKey, m_nKeyLength);
}