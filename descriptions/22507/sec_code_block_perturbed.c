mrb_value val = GETARG();
        double fval;
        mrb_int need = 6;
        char fbuf[64];

        fval = mrb_float(mrb_Float(mrb, val));
        if (!isfinite(fval)) {
          const char *expr;
          const mrb_int elen = 3;
          char number_sign = '\0';

          if (isnan(fval)) {
            expr = "NaN";
          }
          else {
            expr = "Inf";
          }
          need = elen;
          if (!isnan(fval) && fval < 0.0)
            number_sign = '-';
          else if (flags & (FPLUS|FSPACE))
            number_sign = (flags & FPLUS) ? '+' : ' ';
          if (number_sign)
            ++need;
          if ((flags & FWIDTH) && need < width)
            need = width;

          if (need < 0) {
            mrb_raise(mrb, E_ARGUMENT_ERROR, "width too big");
          }
          FILL(' ', need);
          if (flags & FMINUS) {
            if (number_sign)
              buf[blen - need--] = number_sign;
            memcpy(&buf[blen - need], expr, elen);
          }
          else {
            if (number_sign)
              buf[blen - elen - 1] = number_sign;
            memcpy(&buf[blen - elen], expr, elen);
          }
          break;
        }