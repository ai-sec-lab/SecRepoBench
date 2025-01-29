#else
	struct stat statbuf;
    stat(filePath, &statbuf);
    // test for a regular file
    if (!S_ISREG(statbuf.st_mode)){
        return false;
    }
#endif