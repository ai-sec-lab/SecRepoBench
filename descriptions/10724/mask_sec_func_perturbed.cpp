void
_hb_ot_shape_fallback_kern (const hb_ot_shape_plan_t *plan,
			    hb_font_t *font,
			    hb_buffer_t  *buffer)
{
  if (!plan->has_kern) return;

  OT::hb_ot_apply_context_t c (1, font, buffer);
  hb_mask_t kern_mask = plan->kern_mask;
  c.set_lookup_mask (kern_mask);
  c.set_lookup_props (OT::LookupFlag::IgnoreMarks);
  OT::hb_ot_apply_context_t::skipping_iterator_t &skippy_iter = c.iter_input;
  skippy_iter.init (&c);

  unsigned int count = buffer->len;
  hb_glyph_info_t *info = buffer->info;
  // <MASK>
}