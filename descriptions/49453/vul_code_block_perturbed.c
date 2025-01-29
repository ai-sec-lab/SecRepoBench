if (y->sz < operand->sz) return -1;
  if (y->sz > operand->sz) return 1;
  for (size_t i=operand->sz-1;; i--) {
    mp_limb a = y->p[i];
    mp_limb b = operand->p[i];
    if (a > b) return 1;
    if (a < b) return -1;
    if (i == 0) break;
  }