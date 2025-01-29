CPLAssert( m_nKeyLength <= 255 );
    GByte abyKey[255];
    if( m_poDataBlock->ReadBytes(m_nKeyLength, abyKey) != 0 )
        return -1;
    return memcmp(pKeyValue, abyKey, m_nKeyLength);