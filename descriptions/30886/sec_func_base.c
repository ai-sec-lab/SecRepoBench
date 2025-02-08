static void
heredoc_remove_indent(parser_state *p, parser_heredoc_info *hinf)
{
  if (!hinf->remove_indent || hinf->indent == 0)
    return;
  node *indented, *n, *pair, *escaped, *nspaces;
  const char *str;
  size_t len, spaces, offset, start, end;
  indented = hinf->indented;
  while (indented) {
    n = indented->car;
    pair = n->car;
    str = (char*)pair->car;
    len = (size_t)pair->cdr;
    escaped = n->cdr->car;
    nspaces = n->cdr->cdr;
    if (escaped) {
      char *newstr = strndup(str, len);
      size_t newlen = 0;
      start = 0;
      while (start < len) {
        end = escaped ? (size_t)escaped->car : len;
        if (end > len) end = len;
        spaces = (size_t)nspaces->car;
        size_t esclen = end - start;
        heredoc_count_indent(hinf, str + start, esclen, spaces, &offset);
        esclen -= offset;
        memcpy(newstr + newlen, str + start + offset, esclen);
        newlen += esclen;
        start = end;
        if (escaped)
          escaped = escaped->cdr;
        nspaces = nspaces->cdr;
      }
      if (newlen < len)
        newstr[newlen] = '\0';
      pair->car = (node*)newstr;
      pair->cdr = (node*)newlen;
    } else {
      spaces = (size_t)nspaces->car;
      heredoc_count_indent(hinf, str, len, spaces, &offset);
      pair->car = (node*)(str + offset);
      pair->cdr = (node*)(len - offset);
    }
    indented = indented->cdr;
  }
}