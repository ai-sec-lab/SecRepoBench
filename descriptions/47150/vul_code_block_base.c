unsigned char c = path[0];
    unsigned char c1 = 0;
    if (c == joinchar) {
        return TRUE;
    }
    c1 = path[1];
    if (c1 == ':') {
        /*  Windows full path, we assume
            We just assume nobody would be silly enough
            to name a linux/posix directory starting with "C:" */
        return TRUE;
    }