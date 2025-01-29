if (ch == ';' || ch == '\r' || ch == '\n')
            break;
        /* Array protect */
        if (idx==MAXLEN) {
            *next_offset = offset + idx;
            *len = idx;
            return FALSE;
        }