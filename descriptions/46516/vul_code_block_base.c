sb_check (ptr, s->len);
  ptr->len += do_scrub_chars (scrub_from_sb, ptr->ptr + ptr->len, s->len);

  sb_to_scrub = 0;
  scrub_position = 0;