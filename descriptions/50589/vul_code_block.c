    buffer.resize(m_cacheSize);
    if (isEndOfCache(m_cachePos, m_cacheSize) || 0 == m_filePos) {
        if (!readNextBlock()) {
            return false;
        }
    }

    if (IsLineEnd(m_cache[m_cachePos])) {
        // skip line end
        while (m_cache[m_cachePos] != '\n') {
            ++m_cachePos;
        }
        ++m_cachePos;
        if (isEndOfCache(m_cachePos, m_cacheSize)) {
            if (!readNextBlock()) {
                return false;
            }
        }
    }

    size_t i(0);
    while (!IsLineEnd(m_cache[m_cachePos])) {
        buffer[i] = m_cache[m_cachePos];
        ++m_cachePos;
        ++i;

        if(i == buffer.size()) {
            buffer.resize(buffer.size() * 2);
        }

        if (m_cachePos >= m_cacheSize) {
            if (!readNextBlock()) {
                return false;
            }
        }
    }
    buffer[i] = '\n';
    ++m_cachePos;

    return true;