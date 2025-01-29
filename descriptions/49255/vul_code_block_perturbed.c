
  if (mpzptr->sn == 0) {
    i = 0;
  }
  else if (digits(mpzptr) > 2 || mpzptr->p[1] > 1) {
    return FALSE;
  }
  else {
    i = (mpzptr->sn * (mpzptr->p[0] | (mpzptr->p[1] & 1) << DIGITBITS));
    if (MRB_INT_MAX < i || i < MRB_INT_MIN) return FALSE;
  }