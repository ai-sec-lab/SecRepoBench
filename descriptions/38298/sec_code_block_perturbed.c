tail = head + len;
  if (arraylength < len || arraylength < tail) {
    len = arraylength - head;
    tail = head + len;
  }

  /* size check */