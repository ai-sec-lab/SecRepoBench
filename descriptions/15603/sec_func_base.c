static htable*
ht_copy(mrb_state *mrb, htable *t)
{
  segment *seg;
  htable *t2;
  mrb_int i;

  seg = t->rootseg;
  t2 = ht_new(mrb);
  if (t->size == 0) return t2;

  while (seg) {
    for (i=0; i<seg->size; i++) {
      mrb_value key = seg->e[i].key;
      mrb_value val = seg->e[i].val;

      if ((seg->next == NULL) && (i >= t->last_len)) {
        return t2;
      }
      if (mrb_undef_p(key)) continue; /* skip deleted key */
      ht_put(mrb, t2, key, val);
    }
    seg = seg->next;
  }
  return t2;
}