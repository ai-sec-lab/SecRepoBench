static int init_default_huffman_tables(MJpegDecodeContext *s)
{
    static const struct {
        int class;
        int index;
        const uint8_t *bits;
        const uint8_t *values;
        int length;
    } ht[] = {
        { 0, 0, avpriv_mjpeg_bits_dc_luminance,
                avpriv_mjpeg_val_dc, 12 },
        { 0, 1, avpriv_mjpeg_bits_dc_chrominance,
                avpriv_mjpeg_val_dc, 12 },
        { 1, 0, avpriv_mjpeg_bits_ac_luminance,
                avpriv_mjpeg_val_ac_luminance,   162 },
        { 1, 1, avpriv_mjpeg_bits_ac_chrominance,
                avpriv_mjpeg_val_ac_chrominance, 162 },
        { 2, 0, avpriv_mjpeg_bits_ac_luminance,
                avpriv_mjpeg_val_ac_luminance,   162 },
        { 2, 1, avpriv_mjpeg_bits_ac_chrominance,
                avpriv_mjpeg_val_ac_chrominance, 162 },
    };
    int i, ret;

    for (i = 0; i < FF_ARRAY_ELEMS(ht); i++) {
        ret = build_vlc(&s->vlcs[ht[i].class][ht[i].index],
                        ht[i].bits, ht[i].values, ht[i].length,
                        0, ht[i].class == 1);
        if (ret < 0)
            return ret;

        if (ht[i].class < 2) {
            memcpy(s->raw_huffman_lengths[ht[i].class][ht[i].index],
                   ht[i].bits + 1, 16);
            memcpy(s->raw_huffman_values[ht[i].class][ht[i].index],
                   ht[i].values, ht[i].length);
        }
    }

    return 0;
}