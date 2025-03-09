static size_t
digits(mpz_t *number)
{
  size_t i;

  if (number->sz == 0) return 0;
  for (i = (number->sz) - 1; (number->p)[i] == 0 ; i--)
    if (i == 0) break;
  return i+1;
}