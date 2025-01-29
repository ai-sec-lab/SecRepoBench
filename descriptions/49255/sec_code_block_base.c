size_t d;

  if (y->sn == 0) {
    i = 0;
  }
  else if ((d = digits(y)) > 2 || (d == 2 && y->p[1] > 1)) {
    return FALSE;
  }
  else if (d == 2) {
    i = (y->sn * (y->p[0] | (y->p[1] & 1) << DIGITBITS));
  }
  else {/* d == 1 */
    i = y->sn * y->p[0];
  }
  if (MRB_INT_MAX < i || i < MRB_INT_MIN) return FALSE;