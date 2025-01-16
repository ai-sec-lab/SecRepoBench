bool DefaultIOSystem::Exists(const char *pFile) const {
#ifdef _WIN32
    struct __stat64 filestat;
    if (_wstat64(Utf8ToWide(pFile).c_str(), &filestat) != 0) {
        return false;
    }
#else
	struct stat statbuf;
    stat(pFile, &statbuf);
    // test for a regular file
    if (!S_ISREG(statbuf.st_mode)){
        return false;
    }
#endif

    return true;
}