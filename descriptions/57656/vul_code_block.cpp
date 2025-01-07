#ifdef _WIN32
    struct __stat64 filestat;
    if (_wstat64(Utf8ToWide(pFile).c_str(), &filestat) != 0) {
        return false;
    }
#else
    FILE *file = ::fopen(pFile, "rb");
    if (!file) {
        return false;
    }

    ::fclose(file);
#endif