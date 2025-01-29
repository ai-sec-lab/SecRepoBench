if(n->final)
    return ACERR_DUPLICATE_PATTERN;

  n->final = 1;
  node_register_matchstr(n, patt, 0);
  thiz->total_patterns++;

  return ACERR_SUCCESS;