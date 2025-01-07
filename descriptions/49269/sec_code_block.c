
  size_t i;

  if (x->sz == 0) return 0;
  for (i = (x->sz) - 1; (x->p)[i] == 0 ; i--)
    if (i == 0) break;
  return i+1;
