{
  if (utf8) {
    int i;
    for (*cmin = 0, i = 0; (i < cpdmin) && *cmin < len; i++) {
      for ((*cmin)++; *cmin < len && (word[*cmin] & 0xc0) == 0x80; (*cmin)++)
        ;
    }
    for (*cmax = len, i = 0; (i < (cpdmin - 1)) && *cmax >= 0; i++) {
      for ((*cmax)--; *cmax >= 0 && (word[*cmax] & 0xc0) == 0x80; (*cmax)--)
        ;
    }
  } else {
    *cmin = cpdmin;
    *cmax = len - cpdmin + 1;
  }
}