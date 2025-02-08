static int lag_decode_zero_run_line(LagarithContext *l, uint8_t *dst,
                                    const uint8_t *source, const uint8_t *src_end,
                                    int width, int esc_count)
{
    int i = 0;
    int count;
    uint8_t zero_run = 0;
    const uint8_t *src_start = source;
    uint8_t mask1 = -(esc_count < 2);
    uint8_t mask2 = -(esc_count < 3);
    uint8_t *end = dst + (width - 2);

    avpriv_request_sample(l->avctx, "zero_run_line");

    memset(dst, 0, width);

output_zeros:
    if (l->zeros_rem) {
        count = FFMIN(l->zeros_rem, width - i);
        if (end - dst < count) {
            av_log(l->avctx, AV_LOG_ERROR, "Too many zeros remaining.\n");
            return AVERROR_INVALIDDATA;
        }

        memset(dst, 0, count);
        l->zeros_rem -= count;
        dst += count;
    }

    while (dst < end) {
        i = 0;
        while (!zero_run && dst + i < end) {
            i++;
            if (i+2 >= src_end - source)
                return AVERROR_INVALIDDATA;
            zero_run =
                !(source[i] | (source[i + 1] & mask1) | (source[i + 2] & mask2));
        }
        if (zero_run) {
            zero_run = 0;
            i += esc_count;
            if (i >  end - dst ||
                i >= src_end - source)
                return AVERROR_INVALIDDATA;
            memcpy(dst, source, i);
            dst += i;
            l->zeros_rem = lag_calc_zero_run(source[i]);

            source += i + 1;
            goto output_zeros;
        } else {
            memcpy(dst, source, i);
            source += i;
            dst += i;
        }
    }
    return  source - src_start;
}