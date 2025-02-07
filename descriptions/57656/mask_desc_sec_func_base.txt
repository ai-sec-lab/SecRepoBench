bool DefaultIOSystem::Exists(const char *pFile) const {
#ifdef _WIN32
    struct __stat64 filestat;
    if (_wstat64(Utf8ToWide(pFile).c_str(), &filestat) != 0) {
        return false;
    }
// The code checks for the existence of a file at the given path for 
// non-Windows platforms.
// <MASK>

    return true;
}