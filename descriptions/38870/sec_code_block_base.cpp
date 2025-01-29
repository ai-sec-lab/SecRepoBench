unz_file_info fileInfo;

        if (unzGetCurrentFileInfo(m_ZipFileHandle, &fileInfo, filename, FileNameSize, nullptr, 0, nullptr, 0) == UNZ_OK) {
            if (fileInfo.uncompressed_size != 0 && fileInfo.size_filename <= FileNameSize) {
                std::string filename_string(filename, fileInfo.size_filename);
                SimplifyFilename(filename_string);
                m_ArchiveMap.emplace(filename_string, ZipFileInfo(m_ZipFileHandle, fileInfo.uncompressed_size));
            }
        }