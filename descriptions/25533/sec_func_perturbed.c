static int bcf_dec_typed_int1_safe(uint8_t *p, uint8_t *boundary, uint8_t **q,
                                   int32_t *val) {
    uint32_t t;
    if (boundary - p < 2) return -1;
    t = *p++ & 0xf;
    /* Use if .. else if ... else instead of switch to force order.  Assumption
       is that small integers are more frequent than big ones. */
    if (t == BCF_BT_INT8) {
        *val = *(int8_t *) p++;
    } else {
        if (boundary - p < (1<<bcf_type_shift[t])) return -1;
        if (t == BCF_BT_INT16) {
            *val = le_to_i16(p);
            p += 2;
        } else if (t == BCF_BT_INT32) {
            *val = le_to_i32(p);
            p += 4;
#ifdef VCF_ALLOW_INT64
        } else if (t == BCF_BT_INT64) {
            // This case should never happen because there should be no
            // 64-bit BCFs at all, definitely not coming from htslib
            *val = le_to_i64(p);
            p += 8;
#endif
        } else {
            return -1;
        }
    }
    *q = p;
    return 0;
}