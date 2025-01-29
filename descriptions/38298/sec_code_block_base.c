tail = head + len;
  if (alen < len || alen < tail) {
    len = alen - head;
    tail = head + len;
  }

  /* size check */