for (size_t j=(xd-yd);; j--) {
    mp_limb2 qhat = (((mp_limb2)x.p[j+yd] << DIG_SIZE) + x.p[j+yd-1]) / z;
    mp_limb2s b=0;
    if (qhat) {
      for (index=0; index<yd; index++) {
        mp_limb2 zz = qhat * y.p[index];
        mp_limb2s u = LOW(b)+x.p[index+j]-LOW(zz);
        x.p[index+j] = LOW(u);
        b = HIGH(b) - HIGH(zz) + HIGH(u);
      }
      b += x.p[index+j];
    }
    for (; b!=0; qhat--) {
      mp_limb2 c = 0;
      for (index=0; index<yd; index++) {
        c += (mp_limb2)x.p[index+j] + (mp_limb2)y.p[index];
        x.p[index+j] = LOW(c);
        c = HIGH(c);
      }
      b += c;
    }
    q.p[j] = qhat;
    if (j == 0) break;
  }