int   TABINDNode::IndexKeyCmp(GByte *pKeyValue, int entryIndex)
{
    CPLAssert(pKeyValue);
    CPLAssert(entryIndex >= 0 && entryIndex < m_numEntriesInNode);

    m_poDataBlock->GotoByteInBlock(12 + entryIndex*(m_nKeyLength+4));
    CPLAssert( m_nKeyLength <= 255 );
    GByte abyKey[255];
    if( m_poDataBlock->ReadBytes(m_nKeyLength, abyKey) != 0 )
        return -1;
    return memcmp(pKeyValue, abyKey, m_nKeyLength);
}