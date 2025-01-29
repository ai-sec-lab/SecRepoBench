if (coordinate < (double) -SSIZE_MAX)
    return((double) -SSIZE_MAX);
  if (coordinate > (double) SSIZE_MAX)
    return((double) SSIZE_MAX);
  return(coordinate);