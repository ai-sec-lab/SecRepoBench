int VSITarReader::GotoNextFile()
{
#ifdef HAVE_FUZZER_FRIENDLY_ARCHIVE
    if( m_bIsFuzzerFriendly )
    {
        const int nNewFileMarkerSize =
                                static_cast<int>(strlen("***NEWFILE***:"));
        while( true )
        {
            if( m_abyBufferIdx >= m_abyBufferSize )
            {
                if( m_abyBufferSize == 0 )
                {
                    m_abyBufferSize = static_cast<int>(
                        VSIFReadL(m_abyBuffer, 1, 2048, fp));
                    if( m_abyBufferSize == 0 )
                        return FALSE;
                }
                else
                {
                    if( m_abyBufferSize < 2048 )
                    {
                        if( nCurOffset > 0 && nCurOffset != m_nCurOffsetOld )
                        {
                            nNextFileSize = VSIFTellL(fp);
                            nNextFileSize -= m_abyBufferSize;
                            nNextFileSize += m_abyBufferIdx;
                            if( nNextFileSize >= nCurOffset )
                            {
                                nNextFileSize -= nCurOffset;
                                m_nCurOffsetOld = nCurOffset;
                                return TRUE;
                            }
                        }
                        return FALSE;
                    }
                    memcpy(m_abyBuffer, m_abyBuffer + 1024, 1024);
                    m_abyBufferSize = static_cast<int>(
                         VSIFReadL(m_abyBuffer + 1024, 1, 1024, fp));
                    if( m_abyBufferSize == 0 )
                        return FALSE;
                    m_abyBufferIdx = 0;
                    m_abyBufferSize += 1024;
                }
            }
            if( ((m_abyBufferSize == 2048 &&
                  m_abyBufferIdx < m_abyBufferSize -(nNewFileMarkerSize+64)) ||
                 (m_abyBufferSize < 2048 &&
                  m_abyBufferIdx < m_abyBufferSize -(nNewFileMarkerSize+2))) &&
                m_abyBufferIdx >= 0 &&  // Make CSA happy, but useless.
                m_abyBufferIdx < 2048 - nNewFileMarkerSize &&
                memcmp(m_abyBuffer + m_abyBufferIdx,
                       "***NEWFILE***:",
                       strlen("***NEWFILE***:")) == 0 )
            {
                if( nCurOffset > 0 && nCurOffset != m_nCurOffsetOld )
                {
                    nNextFileSize = VSIFTellL(fp);
                    nNextFileSize -= m_abyBufferSize;
                    nNextFileSize += m_abyBufferIdx;
                    if( nNextFileSize >= nCurOffset )
                    {
                        nNextFileSize -= nCurOffset;
                        m_nCurOffsetOld = nCurOffset;
                        return TRUE;
                    }
                }
                m_abyBufferIdx += nNewFileMarkerSize;
                const int nFilenameStartIdx = m_abyBufferIdx;
                for( ; m_abyBufferIdx < m_abyBufferSize &&
                       m_abyBuffer[m_abyBufferIdx] != '\n';
                     ++m_abyBufferIdx)
                {
                    // Do nothing.
                }
                if( m_abyBufferIdx < m_abyBufferSize )
                {
                    osNextFileName.assign(
                        (const char*)(m_abyBuffer + nFilenameStartIdx),
                        m_abyBufferIdx - nFilenameStartIdx);
                    nCurOffset = VSIFTellL(fp);
                    nCurOffset -= m_abyBufferSize;
                    nCurOffset += m_abyBufferIdx + 1;
                }
            }
            else
            {
                m_abyBufferIdx++;
            }
        }
    }
#endif
    char abyHeader[512] = {};
    if (VSIFReadL(abyHeader, 512, 1, fp) != 1)
        return FALSE;

    if (abyHeader[99] != '\0' ||
        abyHeader[107] != '\0' ||
        abyHeader[115] != '\0' ||
        abyHeader[123] != '\0' ||
        (abyHeader[135] != '\0' && abyHeader[135] != ' ') ||
        (abyHeader[147] != '\0' && abyHeader[147] != ' '))
    {
        return FALSE;
    }
    if( abyHeader[124] < '0' || abyHeader[124] > '7' )
        return FALSE;

    osNextFileName = abyHeader;
    nNextFileSize = 0;
    for(int index=0;index<11;index++)
        nNextFileSize = nNextFileSize * 8 + (abyHeader[124+index] - '0');
    // <MASK>
}