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
    mpz_set(mrb, rr, xx);
    return;
  }

  mpz_t q, x, y;
  size_t i;

  mrb_assert(!uzero(yy));       /* divided by zero */
  mpz_init(mrb, &q);
  mpz_init(mrb, &x);
  mpz_init(mrb, &y);
  mpz_realloc(mrb, &x, xx->sz+1);
  size_t ydigits = digits(yy);
  size_t ns = lzb(yy->p[ydigits-1]);
  ulshift(mrb, &x, xx, ns);
  ulshift(mrb, &y, yy, ns);
  size_t xd = digits(&x);
  mpz_realloc(mrb, &q, xd);
  mp_dbl_limb z = y.p[ydigits-1];
  // This code block implements the division of multi-precision integers using a long division algorithm.
  // It computes the quotient (q) and updates the remainder (x) by iterating over the digits of the dividend.
  // For each digit, a trial quotient (qhat) is calculated and used to adjust the remainder.
  // It handles multi-digit divisions by scaling and adjusting the operands accordingly.
  // The loop iterates over the difference in digit count between the dividend and divisor, updating the quotient q.
  // <MASK>
  x.sz = yy->sz;
  urshift(mrb, rr, &x, ns);
  trim(&q);
  mpz_move(mrb, qq, &q);
  mpz_clear(mrb, &x);
  mpz_clear(mrb, &y);
}