static void
udiv(mrb_state *mrb, mpz_t *qq, mpz_t *rr, mpz_t *xx, mpz_t *yy)
{
  /* simple cases */
  int cmp = ucmp(xx, yy);
  if (cmp == 0) {
    mpz_set_int(mrb, qq, 1);
    zero(rr);
    return;
  }
  else if (cmp < 0) {
    zero(qq);
    mpz_set(mrb, rr, yy);
    return;
  }

  mpz_t q, x, y;
  size_t index;

  mrb_assert(!uzero(yy));       /* divided by zero */
  mpz_init(mrb, &q);
  mpz_init(mrb, &x);
  mpz_init(mrb, &y);
  mpz_realloc(mrb, &x, xx->sz+1);
  size_t yd = digits(yy);
  size_t ns = lzb(yy->p[yd-1]);
  ulshift(mrb, &x, xx, ns);
  ulshift(mrb, &y, yy, ns);
  size_t xd = digits(&x);
  mpz_realloc(mrb, &q, xd);
  mp_limb2 z = y.p[yd-1];
  // Perform long division of the unsigned integers represented by `xx` and `yy`.
  // <MASK>
  urshift(mrb, rr, &x, ns);
  trim(&q);
  mpz_set(mrb, qq, &q);
  mpz_clear(mrb, &x);
  mpz_clear(mrb, &y);
  mpz_clear(mrb, &q);
}