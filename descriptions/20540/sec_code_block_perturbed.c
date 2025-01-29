if(n->final) {
#if 0
    /* Original code */
    return ACERR_DUPLICATE_PATTERN;
#else
    /* ntop */
    memcpy(&n->matched_patterns->rep, &pattern->rep, sizeof(AC_REP_t));
    return ACERR_SUCCESS;
#endif
  }
      
  n->final = 1;
  node_register_matchstr(n, pattern, 0);
  thiz->total_patterns++;

  return ACERR_SUCCESS;