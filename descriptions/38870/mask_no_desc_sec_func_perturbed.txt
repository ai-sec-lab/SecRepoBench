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
        char filename[FileNameSize];
        // <MASK>
    } while (unzGoToNextFile(m_ZipFileHandle) != UNZ_END_OF_LIST_OF_FILE);
}