begin = 0;
      while (begin < len) {
        end = escaped ? (size_t)escaped->car : len;
        spaces = (size_t)nspaces->car;
        size_t esclen = end - begin;
        heredoc_count_indent(hinf, str + begin, esclen, spaces, &offset);
        esclen -= offset;
        memcpy(newstr + newlen, str + begin + offset, esclen);
        newlen += esclen;
        begin = end;
        if (escaped)
          escaped = escaped->cdr;
        nspaces = nspaces->cdr;
      }
      if (newlen < len)
        newstr[newlen] = '\0';
      pair->car = (node*)newstr;
      pair->cdr = (node*)newlen;