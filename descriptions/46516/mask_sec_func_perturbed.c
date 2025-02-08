void
sb_scrub_and_add_sb (sb *ptr, sb *scrubbuffer)
{
  sb_to_scrub = scrubbuffer;
  scrub_position = scrubbuffer->ptr;

  // <MASK>
}