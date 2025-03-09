static int
mpz_get_int(mpz_t *mpzptr, mrb_int *v)
{
  mp_limb i;
  size_t d;

  if (mpzptr->sn == 0) {
    i = 0;
  }
  else if ((d = digits(mpzptr)) > 2 || (d == 2 && mpzptr->p[1] > 1)) {
    return FALSE;
  }
  else if (d == 2) {
    i = (mpzptr->sn * (mpzptr->p[0] | (mpzptr->p[1] & 1) << DIGITBITS));
  }
  else {/* d == 1 */
    i = mpzptr->sn * mpzptr->p[0];
  }
  if (MRB_INT_MAX < i || i < MRB_INT_MIN) return FALSE;
  *v = i;
  return TRUE;
}