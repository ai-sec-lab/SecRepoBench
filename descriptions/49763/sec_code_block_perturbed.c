if (xd>=ydigits) {
    for (size_t j=xd-ydigits;; j--) {
      mp_dbl_limb_signed b=0;
      mp_dbl_limb qhat;

      if (j+ydigits == xd)
        qhat = x.p[j+ydigits-1] / z;
      else
        qhat = (((mp_dbl_limb)x.p[j+ydigits] << DIG_SIZE) + x.p[j+ydigits-1]) / z;
      if (qhat) {
        for (i=0; i<ydigits; i++) {
          mp_dbl_limb zz = qhat * y.p[i];
          mp_dbl_limb_signed u = LOW(b)+x.p[i+j]-LOW(zz);
          x.p[i+j] = LOW(u);
          b = HIGH(b) - HIGH(zz) + HIGH(u);
        }
        b += x.p[i+j];
      }
      for (; b!=0; qhat--) {
        mp_dbl_limb c = 0;
        for (i=0; i<ydigits; i++) {
          c += (mp_dbl_limb)x.p[i+j] + (mp_dbl_limb)y.p[i];
          x.p[i+j] = LOW(c);
          c = HIGH(c);
        }
        b += c;
      }
      q.p[j] = (mp_limb)qhat;
      if (j == 0) break;
    }
  }
  x.sz = yy->sz;
  urshift(mrb, rr, &x, ns);
  trim(&q);
  mpz_move(mrb, qq, &q);
  mpz_clear(mrb, &x);
  mpz_clear(mrb, &y);