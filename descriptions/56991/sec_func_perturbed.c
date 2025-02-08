MRB_API void
mrb_field_write_barrier(mrb_state *mrbstateptr, struct RBasic *obj, struct RBasic *value)
{
  mrb_gc *gc = &mrbstateptr->gc;

  if (!value) return;
  if (!is_black(obj)) return;
  if (!is_white(value)) return;
  if (is_red(value)) return;

  mrb_assert(gc->state == MRB_GC_STATE_MARK || (!is_dead(gc, value) && !is_dead(gc, obj)));
  mrb_assert(is_generational(gc) || gc->state != MRB_GC_STATE_ROOT);

  if (gc->state == MRB_GC_STATE_SWEEP) {
    paint_partial_white(gc, obj); /* for never write barriers */
  }
  else {
    add_gray_list(mrbstateptr, gc, value);
  }
}