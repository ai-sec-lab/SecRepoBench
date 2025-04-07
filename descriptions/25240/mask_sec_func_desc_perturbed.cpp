void ObjFileMtlImporter::load() {
    if (m_DataIt == m_DataItEnd)
        return;

    while (m_DataIt != m_DataItEnd) {
        switch (*m_DataIt) {
            case 'k':
            // Parses material properties from a buffer when loading material files. The code processes
            // different material properties based on the current character in the buffer. It handles:
            // 1. Colors: Ambient, Diffuse, Specular, Emissive, and Transmission.
            // 2. Transparency and Alpha values.
            // 3. Specular exponent and Index of Refraction (IOR).
            // 4. Materials: Creates a new material when needed.
            // 5. Textures: Handles various texture types like diffuse, bump, etc.
            // 6. Illumination models and anisotropy.
            // The parsing is done by moving through the buffer character by character, determining
            // which property is being specified, and calling the appropriate function to parse
            // and assign the value to the current material being processed.
            // <MASK>

            default: {
                m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            } break;
        }
    }
}