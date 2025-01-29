#else
    FILE *file = ::fopen(filePath, "rb");
    if (!file) {
        return false;
    }

    ::fclose(file);
#endif