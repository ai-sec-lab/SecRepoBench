{
    int l1 = strlen(source);
    int l2 = strlen(s2);
    if (l1 <= l2 && s2[l1 - 1] == s2[l2 - 1])
      return 1;
  } else if (csconv) {
    const char* olds = source;
    // decapitalise dictionary word
    if ((*source != *s2) && (*source != csconv[((unsigned char)*s2)].clower))
      return 0;
    do {
      source++;
      s2++;
    } while ((*source == *s2) && (*source != '\0'));
    return (int)(source - olds);
  }