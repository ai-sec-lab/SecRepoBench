size_t xd = digits(&x);
  mpz_realloc(mrb, &q, xd);
  mp_limb2 z = y.p[yd-1];
  for (size_t j=xd-yd;; j--) {
    mp_limb2 qhat = (((mp_limb2)x.p[j+yd] << DIG_SIZE) + x.p[j+yd-1]) / z;
    mp_limb2s b=0;
    if (qhat) {
      for (i=0; i<yd; i++) {
        mp_limb2 zz = qhat * y.p[i];
        mp_limb2s u = LOW(b)+x.p[i+j]-LOW(zz);
        x.p[i+j] = LOW(u);
        b = HIGH(b) - HIGH(zz) + HIGH(u);
      }
      b += x.p[i+j];
    }
    for (; b!=0; qhat--) {
      mp_limb2 c = 0;
      for (i=0; i<yd; i++) {
        c += (mp_limb2)x.p[i+j] + (mp_limb2)y.p[i];
        x.p[i+j] = LOW(c);
        c = HIGH(c);
      }
      b += c;
    }
    q.p[j] = qhat;
    if (j == 0) break;
  }