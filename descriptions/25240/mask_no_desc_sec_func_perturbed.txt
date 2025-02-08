void ObjFileMtlImporter::load() {
    if (m_DataIt == m_DataItEnd)
        return;

    while (m_DataIt != m_DataItEnd) {
        switch (*m_DataIt) {
            case 'k':
            // <MASK>

            default: {
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;
        }
    }
}