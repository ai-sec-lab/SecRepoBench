static void
udiv(mrb_state *mrbstateptr, mpz_t *qq, mpz_t *rr, mpz_t *xx, mpz_t *yy)
{
  /* simple cases */
  int cmp = ucmp(xx, yy);
  if (cmp == 0) {
    mpz_set_int(mrbstateptr, qq, 1);
    zero(rr);
    return;
  }
  else if (cmp < 0) {
    zero(qq);
    mpz_set(mrbstateptr, rr, yy);
    return;
  }

  mpz_t q, x, y;
  size_t i;

  mrb_assert(!uzero(yy));       /* divided by zero */
  mpz_init(mrbstateptr, &q);
  mpz_init(mrbstateptr, &x);
  mpz_init(mrbstateptr, &y);
  mpz_realloc(mrbstateptr, &x, xx->sz+1);
  size_t yd = digits(yy);
  size_t ns = lzb(yy->p[yd-1]);
  ulshift(mrbstateptr, &x, xx, ns);
  ulshift(mrbstateptr, &y, yy, ns);
  // <MASK>
  x.sz = yy->sz;
  urshift(mrbstateptr, rr, &x, ns);
  trim(&q);
  mpz_set(mrbstateptr, qq, &q);
  mpz_clear(mrbstateptr, &x);
  mpz_clear(mrbstateptr, &y);
  mpz_clear(mrbstateptr, &q);
}