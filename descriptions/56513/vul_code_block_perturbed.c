{
    num_digits = prec + 1;
  } else if (formatchar == 'g') {
    if (prec == 0) {
      prec = 1;
    }
    num_digits = prec;
  }