memcpy(dst, src, i);
            dst += i;
            l->zeros_rem = lag_calc_zero_run(src[i]);

            src += i + 1;
            goto output_zeros;