size_t i;

    char *disk_filename = (char*)malloc(io_stream->m_Filename.length() + 1);
    strncpy(disk_filename, io_stream->m_Filename.c_str(), io_stream->m_Filename.length() + 1);
    for (i = io_stream->m_Filename.length() - 1; i >= 0; i -= 1)
    {
        if (disk_filename[i] != '.')
            continue;
        snprintf(&disk_filename[i], io_stream->m_Filename.length() - i, ".z%02u", number_disk + 1);
        break;
    }