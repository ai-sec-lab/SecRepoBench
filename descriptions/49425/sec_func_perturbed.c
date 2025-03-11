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
  for (size_t j=xd-yd-1;; j--) {
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
  urshift(mrb, rr, &x, ns);
  trim(&q);
  mpz_set(mrb, qq, &q);
  mpz_clear(mrb, &x);
  mpz_clear(mrb, &y);
  mpz_clear(mrb, &q);
}