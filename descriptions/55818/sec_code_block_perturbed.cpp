if (utf8) {
    int i;
    for (*mincharindex = 0, i = 0; (i < cpdmin) && *mincharindex < len; i++) {
      for ((*mincharindex)++; *mincharindex < len && (word[*mincharindex] & 0xc0) == 0x80; (*mincharindex)++)
        ;
    }
    for (*cmax = len, i = 0; (i < (cpdmin - 1)) && *cmax > 0; i++) {
      for ((*cmax)--; *cmax > 0 && (word[*cmax] & 0xc0) == 0x80; (*cmax)--)
        ;
    }
  } else {
    *mincharindex = cpdmin;
    *cmax = len - cpdmin + 1;
  }