if (i >  end - dst ||
                i >= src_end - source)
                return AVERROR_INVALIDDATA;
            memcpy(dst, source, i);
            dst += i;
            l->zeros_rem = lag_calc_zero_run(source[i]);

            source += i + 1;
            goto output_zeros;