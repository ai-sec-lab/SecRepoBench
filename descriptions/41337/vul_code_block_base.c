*idx += b * 8;
  r = xcalloc (b + 1, 1);
  memcpy (r, ptr + oc, b);
  r[b] = 0;

  return r;