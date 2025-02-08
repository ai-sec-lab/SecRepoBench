static htable*
ht_copy(mrb_state *mrbstateptr, htable *t)
{
  segment *seg;
  htable *t2;
  mrb_int i;

  seg = t->rootseg;
  t2 = ht_new(mrbstateptr);
  if (t->size == 0) return t2;

  while (seg) {
    for (i=0; i<seg->size; i++) {
      mrb_value key = seg->e[i].key;
      mrb_value val = seg->e[i].val;

      // <MASK>
    }
    seg = seg->next;
  }
  return t2;
}