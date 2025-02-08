bool DefaultIOSystem::Exists(const char *pFile) const {
#ifdef _WIN32
    struct __stat64 filestat;
    if (_wstat64(Utf8ToWide(pFile).c_str(), &filestat) != 0) {
        return false;
    }
// <MASK>

    return true;
}