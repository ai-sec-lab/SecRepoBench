const FWORD *v = &StructAtOffset<FWORD> (&(this+array), offset);
    /* Untested code, as I have not been able to find ANY kern table format-2 yet. */
    assert (&(this+array) <= v);
    if (unlikely (v + 1 > (const FWORD *) end))
      return 0;