if (!c) {
        /* empty string. */
        return FALSE;
    }
    if (c == joinchar) {
        return TRUE;
    }
    secondchar = path[1];
    if (!secondchar) {
        /* No second character */
        return FALSE;
    }