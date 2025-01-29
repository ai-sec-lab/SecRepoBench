#else
    FILE *file = ::fopen(pFile, "rb");
    if (!file) {
        return false;
    }

    ::fclose(file);
#endif