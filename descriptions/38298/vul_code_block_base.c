tail = head + len;
  if (alen < len || alen < tail) {
    len = alen - head;
  }

  /* size check */