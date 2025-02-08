int   TABINDNode::IndexKeyCmp(GByte *pKeyValue, int nEntryNo)
{
    CPLAssert(pKeyValue);
    CPLAssert(nEntryNo >= 0 && nEntryNo < m_numEntriesInNode);

    m_poDataBlock->GotoByteInBlock(12 + nEntryNo*(m_nKeyLength+4));
    // <MASK>
}