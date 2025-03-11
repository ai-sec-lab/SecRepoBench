static int
mpz_get_int(mpz_t *mpzptr, mrb_int *v)
{
  mp_limb i;
  // This code block is responsible for converting a multi-precision integer
  // (represented by mpz_t structure) to a single precision integer (mrb_int),
  // if possible. If conversion is not possible, return FALSE.
  // <MASK>
  *v = i;
  return TRUE;
}