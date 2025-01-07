
        mrb_value val = GETARG();
        double fval;
        mrb_int need = 6;
        char fbuf[32];

        fval = mrb_float(mrb_Float(mrb, val));
        if (!isfinite(fval)) {
          const char *expr;
          const mrb_int elen = 3;
          char sign = '\0';

          if (isnan(fval)) {
            expr = "NaN";
          }
          else {
            expr = "Inf";
          }
          need = elen;
          if (!isnan(fval) && fval < 0.0)
            sign = '-';
          else if (flags & (FPLUS|FSPACE))
            sign = (flags & FPLUS) ? '+' : ' ';
          if (sign)
            ++need;
          if ((flags & FWIDTH) && need < width)
            need = width;

          if (need < 0) {
            mrb_raise(mrb, E_ARGUMENT_ERROR, "width too big");
          }
          FILL(' ', need);
          if (flags & FMINUS) {
            if (sign)
              buf[blen - need--] = sign;
            memcpy(&buf[blen - need], expr, elen);
          }
          else {
            if (sign)
              buf[blen - elen - 1] = sign;
            memcpy(&buf[blen - elen], expr, elen);
          }
          break;
        }

        fmt_setup(fbuf, sizeof(fbuf), *p, flags, width, prec);
        need = 0;
        if (*p != 'e' && *p != 'E') {
          int i;
          frexp(fval, &i);
          if (i > 0)
            need = BIT_DIGITS(i);
        }
        if (need > MRB_INT_MAX - ((flags&FPREC) ? prec : 6)) {
        too_big_width:
          mrb_raise(mrb, E_ARGUMENT_ERROR,
                    (width > prec ? "width too big" : "prec too big"));
        }
        need += (flags&FPREC) ? prec : 6;
        if ((flags&FWIDTH) && need < width)
          need = width;
        if (need > MRB_INT_MAX - 20) {
          goto too_big_width;
        }
        need += 20;

        CHECK(need);
        n = mrb_float_to_cstr(mrb, &buf[blen], need, fbuf, fval);
        if (n < 0 || n >= need) {
          mrb_raise(mrb, E_RUNTIME_ERROR, "formatting error");
        }
        blen += n;
      