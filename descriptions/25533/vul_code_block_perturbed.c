{
        *val = *(int8_t *) p++;
    } else {
        if (boundary - p < bcf_type_shift[t]) return -1;
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