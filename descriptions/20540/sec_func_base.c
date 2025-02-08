AC_ERROR_t ac_automata_add (AC_AUTOMATA_t * thiz, AC_PATTERN_t * patt)
{
  unsigned int i;
  AC_NODE_t * n = thiz->root;
  AC_NODE_t * next;
  AC_ALPHABET_t alpha;

  if(!thiz->automata_open)
    return ACERR_AUTOMATA_CLOSED;

  if (!patt->length)
    return ACERR_ZERO_PATTERN;

  if (patt->length > AC_PATTRN_MAX_LENGTH)
    return ACERR_LONG_PATTERN;

  for (i=0; i<patt->length; i++)
  {
    alpha = patt->astring[i];
    if ((next = node_find_next(n, alpha)))
    {
      n = next;
      continue;
    }
    else
    {
      next = node_create_next(n, alpha);
      next->depth = n->depth + 1;
      n = next;
      ac_automata_register_nodeptr(thiz, n);
    }
  }

  if(n->final) {
#if 0
    /* Original code */
    return ACERR_DUPLICATE_PATTERN;
#else
    /* ntop */
    memcpy(&n->matched_patterns->rep, &patt->rep, sizeof(AC_REP_t));
    return ACERR_SUCCESS;
#endif
  }
      
  n->final = 1;
  node_register_matchstr(n, patt, 0);
  thiz->total_patterns++;

  return ACERR_SUCCESS;
}