AC_ERROR_t ac_automata_add (AC_AUTOMATA_t * thiz, AC_PATTERN_t * pattern)
{
  unsigned int i;
  AC_NODE_t * n = thiz->root;
  AC_NODE_t * next;
  AC_ALPHABET_t alpha;

  if(!thiz->automata_open)
    return ACERR_AUTOMATA_CLOSED;

  if (!pattern->length)
    return ACERR_ZERO_PATTERN;

  if (pattern->length > AC_PATTRN_MAX_LENGTH)
    return ACERR_LONG_PATTERN;

  for (i=0; i<pattern->length; i++)
  {
    alpha = pattern->astring[i];
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

  // <MASK>
}