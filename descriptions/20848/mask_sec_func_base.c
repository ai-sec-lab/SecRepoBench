bfd_boolean
bfd_check_format_matches (bfd *abfd, bfd_format format, char ***matching)
{
  extern const bfd_target binary_vec;
#if BFD_SUPPORTS_PLUGINS
  extern const bfd_target plugin_vec;
#endif
  const bfd_target * const *target;
  const bfd_target **matching_vector = NULL;
  const bfd_target *save_targ, *right_targ, *ar_right_targ, *match_targ;
  int match_count, best_count, best_match;
  int ar_match_index;
  unsigned int initial_section_id = _bfd_section_id;
  // <MASK>
  return FALSE;
}