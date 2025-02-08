
  if (y->sn == 0) {
    i = 0;
  }
  else if (digits(y) > 2 || y->p[1] > 1) {
    return FALSE;
  }
  else {
    i = (y->sn * (y->p[0] | (y->p[1] & 1) << DIGITBITS));
    if (MRB_INT_MAX < i || i < MRB_INT_MIN) return FALSE;
  }