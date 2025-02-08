int   TABINDNode::IndexKeyCmp(GByte *pKeyValue, int entryIndex)
{
    CPLAssert(pKeyValue);
    CPLAssert(entryIndex >= 0 && entryIndex < m_numEntriesInNode);

    m_poDataBlock->GotoByteInBlock(12 + entryIndex*(m_nKeyLength+4));
    // <MASK>
}