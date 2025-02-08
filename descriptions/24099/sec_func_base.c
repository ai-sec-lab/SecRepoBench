static gboolean get_next_item(tvbuff_t *tvb, gint offset, gint maxlen, guint8 *str, gint *next_offset, guint *len)
{
    guint  idx = 0;
    guint8 ch;

    /* Obtain items */
    while (maxlen > 1) {
        ch = tvb_get_guint8(tvb, offset+idx);
        if (ch == ';' || ch == '\r' || ch == '\n')
            break;
        /* Array protect */
        if (idx == MAXLEN - 1) {
            *next_offset = offset + idx;
            *len = idx;
            return FALSE;
        }
        /* Copy data into string array */
        str[idx++] = ch;
        maxlen--;
    }
    /* Null terminate the item */
    str[idx] = '\0';

    /* Update admin for next item */
    *next_offset = offset + idx;
    *len = idx;

    return TRUE;
}