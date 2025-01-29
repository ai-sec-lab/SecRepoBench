{
    num_digits = prec + 1;
    if (prec == 0) prec = 1;
  }
  else if (fmt == 'g') {
    num_digits = prec;
  }