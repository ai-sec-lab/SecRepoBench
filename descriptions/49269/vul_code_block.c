size_t i;
  for (i = (x->sz) - 1; (x->p)[i] == 0 ; i--)
    if (i == 0) break;
  return i+1;