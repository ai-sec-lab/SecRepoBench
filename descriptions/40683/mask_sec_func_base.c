static void
evax_bfd_print_dst (struct bfd *abfd, unsigned int dst_size, FILE *file)
{
  unsigned int off = 0;
  unsigned int pc = 0;
  unsigned int line = 0;

  fprintf (file, _("Debug symbol table:\n"));

  while (dst_size > 0)
    {
      struct vms_dst_header dsth;
      unsigned int len;
      unsigned int type;
      unsigned char *buf;

      if (bfd_bread (&dsth, sizeof (dsth), abfd) != sizeof (dsth))
	{
	  fprintf (file, _("cannot read DST header\n"));
	  return;
	}
      len = bfd_getl16 (dsth.length);
      type = bfd_getl16 (dsth.type);
      /* xgettext:c-format */
      fprintf (file, _(" type: %3u, len: %3u (at 0x%08x): "),
	       type, len, off);
      // <MASK>
      free (buf);
    }
}