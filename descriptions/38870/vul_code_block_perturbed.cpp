unz_file_info filemetadata;

        if (unzGetCurrentFileInfo(m_ZipFileHandle, &filemetadata, filename, FileNameSize, nullptr, 0, nullptr, 0) == UNZ_OK) {
            if (filemetadata.uncompressed_size != 0) {
                std::string filename_string(filename, filemetadata.size_filename);
                SimplifyFilename(filename_string);
                m_ArchiveMap.emplace(filename_string, ZipFileInfo(m_ZipFileHandle, filemetadata.uncompressed_size));
            }
        }