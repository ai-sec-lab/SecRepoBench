void
sb_scrub_and_add_sb (sb *ptr, sb *scrubbuffer)
{
  sb_to_scrub = scrubbuffer;
  scrub_position = scrubbuffer->ptr;

  /* do_scrub_chars can expand text, for example when replacing
     # 123 "filename"
     with
     \t.linefile 123 "filename"
     or when replacing a 'c with the decimal ascii number for c.
     So we loop until the input S is consumed.  */
  while (1)
    {
      size_t copy = scrubbuffer->len - (scrub_position - scrubbuffer->ptr);
      if (copy == 0)
	break;
      sb_check (ptr, copy);
      ptr->len += do_scrub_chars (scrub_from_sb, ptr->ptr + ptr->len, copy);
    }

  sb_to_scrub = 0;
  scrub_position = 0;
}