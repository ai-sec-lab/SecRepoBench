tail = head + len;
  if (arraylength < len || arraylength < tail) {
    len = arraylength - head;
  }

  /* size check */