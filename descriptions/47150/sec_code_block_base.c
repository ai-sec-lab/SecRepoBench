if (!c) {
        /* empty string. */
        return FALSE;
    }
    if (c == joinchar) {
        return TRUE;
    }
    c1 = path[1];
    if (!c1) {
        /* No second character */
        return FALSE;
    }