wmem_strbuf_t *str;
    const guint8 *prev = ptr;
    size_t valid_bytes = 0;

    str = wmem_strbuf_new_sized(scope, length+1);

    /* See the Unicode Standard conformance chapter at
     * https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf especially
     * Table 3-7 "Well-Formed UTF-8 Byte Sequences" and
     * U+FFFD Substitution of Maximal Subparts. */

    while (length > 0) {

        prev = ptr;
        valid_bytes = utf_8_validate(prev, length, &ptr);
        if (valid_bytes) {
            wmem_strbuf_append_len(str, prev, valid_bytes);
        }
        length -= (gint)valid_bytes;
        if (length) {
            wmem_strbuf_append_unichar_repl(str);
        }
    }