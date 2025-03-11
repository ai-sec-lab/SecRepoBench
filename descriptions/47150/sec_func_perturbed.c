static int
is_full_path(char *file_path,int joinchar)
{
    unsigned char c = file_path[0];
    unsigned char c1 = 0;
    if (!c) {
        /* empty string. */
        return FALSE;
    }
    if (c == joinchar) {
        return TRUE;
    }
    c1 = file_path[1];
    if (!c1) {
        /* No second character */
        return FALSE;
    }
    if (c1 == ':') {
        /*  Windows full path, we assume
            We just assume nobody would be silly enough
            to name a linux/posix directory starting with "C:" */
        return TRUE;
    }
    /*  No kind of full path name */
    return FALSE;
}