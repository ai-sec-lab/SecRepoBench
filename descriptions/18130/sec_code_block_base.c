if (x < (double) -(SSIZE_MAX-512))
    return((double) -(SSIZE_MAX-512));
  if (x > (double) (SSIZE_MAX-512))
    return((double) (SSIZE_MAX-512));
  return(x);