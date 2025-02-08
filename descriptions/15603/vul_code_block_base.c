      if (mrb_undef_p(key)) continue; /* skip deleted key */
      if ((seg->next == NULL) && (i >= t->last_len)) {
        return t2;
      }
      ht_put(mrb, t2, key, val);