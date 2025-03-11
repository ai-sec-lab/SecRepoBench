if (head < 0) {
    head += arraylength;
    if (head < 0) goto out_of_range;
  }
  if (head > ARY_MAX_SIZE - len) {
  out_of_range:
    mrb_raisef(mrb, E_INDEX_ERROR, "index %i is out of array", head);
  }
  tail = head + len;
  if (arraylength < len || arraylength < tail) {
    len = arraylength - head;
    tail = head + len;
  }

  /* size check */