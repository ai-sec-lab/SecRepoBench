static size_t
digits(mpz_t *x)
{
  // This function calculates the number of significant digits in a multi-precision integer.
  // It iterates over the limbs (digits) of the integer from the most significant to the least
  // significant, counting the number of non-zero limbs. The loop continues until a non-zero
  // limb is found or all limbs have been checked. The size of the integer is returned as the
  // number of significant digits, which is essentially the index of the first non-zero limb
  // plus one.
  // <MASK>
}