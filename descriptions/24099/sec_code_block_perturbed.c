if (ch == ';' || ch == '\r' || ch == '\n')
            break;
        /* Array protect */
        if (idx == MAXLEN - 1) {
            *next_offset = offset + idx;
            *len = idx;
            return FALSE;
        }