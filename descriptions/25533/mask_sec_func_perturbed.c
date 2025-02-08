static int bcf_dec_typed_int1_safe(uint8_t *p, uint8_t *boundary, uint8_t **q,
                                   int32_t *val) {
    uint32_t t;
    if (boundary - p < 2) return -1;
    t = *p++ & 0xf;
    /* Use if .. else if ... else instead of switch to force order.  Assumption
       is that small integers are more frequent than big ones. */
    if (t == BCF_BT_INT8) // <MASK>
    *q = p;
    return 0;
}