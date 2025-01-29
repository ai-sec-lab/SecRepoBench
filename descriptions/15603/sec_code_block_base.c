if ((seg->next == NULL) && (i >= t->last_len)) {
        return t2;
      }
      if (mrb_undef_p(key)) continue; /* skip deleted key */
      ht_put(mrb, t2, key, val);