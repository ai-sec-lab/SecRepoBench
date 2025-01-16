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
        unz_file_info fileInfo;

        if (unzGetCurrentFileInfo(m_ZipFileHandle, &fileInfo, filename, FileNameSize, nullptr, 0, nullptr, 0) == UNZ_OK) {
            if (fileInfo.uncompressed_size != 0 && fileInfo.size_filename <= FileNameSize) {
                std::string filename_string(filename, fileInfo.size_filename);
                SimplifyFilename(filename_string);
                m_ArchiveMap.emplace(filename_string, ZipFileInfo(m_ZipFileHandle, fileInfo.uncompressed_size));
            }
        }
    } while (unzGoToNextFile(m_ZipFileHandle) != UNZ_END_OF_LIST_OF_FILE);
}