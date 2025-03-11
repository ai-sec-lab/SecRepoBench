void ZipArchiveIOSystem::Implement::MapArchive() {
    if (m_ZipFileHandle == nullptr)
        return;

    if (!m_ArchiveMap.empty())
        return;

    //  At first ensure file is already open
    if (unzGoToFirstFile(m_ZipFileHandle) != UNZ_OK)
        return;

    // Loop over all files
    do {
        char filename[max_filename_length];
        // Retrieves information for the current file within the ZIP archive. It checks the file's uncompressed size
        // to ensure it's not zero and constructs a filename string using the provided buffer. The filename is then
        // simplified by removing unnecessary components. The simplified filename is used as a key to map the 
        // file information in the archive map, using the filename as the key and a ZipFileInfo instance containing
        // the file handle and uncompressed size as the value.
        // <MASK>
    } while (unzGoToNextFile(m_ZipFileHandle) != UNZ_END_OF_LIST_OF_FILE);
}