unsigned int len;

  if (maxlen == 0)
    return NULL;
  len = *ptr++;
  if (len >  maxlen - 1)
    return NULL;