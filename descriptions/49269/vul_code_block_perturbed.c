size_t i;
  for (i = (number->sz) - 1; (number->p)[i] == 0 ; i--)
    if (i == 0) break;
  return i+1;