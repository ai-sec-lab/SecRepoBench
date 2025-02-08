static void context_closure_recurse_lookups (hb_closure_context_t *c,
					     unsigned inputCount, const HBUINT16 input[],
					     unsigned lookupCount,
					     const LookupRecord lookupRecord[] /* Array of LookupRecords--in design order */,
					     unsigned value,
					     ContextFormat context_format,
					     const void *data,
					     intersected_glyphs_func_t intersected_glyphs_func)
{
  hb_set_t *covered_seq_indicies = hb_set_create ();
  for (unsigned int i = 0; i < lookupCount; i++// <MASK>

  hb_set_destroy (covered_seq_indicies);
}