{
    int l1 = strlen(s1);
    int l2 = strlen(s2);
    if (l1 && l1 <= l2 && s2[l1 - 1] == s2[l2 - 1])
      return 1;
  } else if (csconv) {
    const char* olds = s1;
    // decapitalise dictionary word
    if ((*s1 != *s2) && (*s1 != csconv[((unsigned char)*s2)].clower))
      return 0;
    do {
      s1++;
      s2++;
    } while ((*s1 == *s2) && (*s1 != '\0'));
    return (int)(s1 - olds);
  }