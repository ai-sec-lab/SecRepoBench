static int
suffix (struct buffer *buf, disassemble_info *info, const char *suffixtext)
{
  char mybuf[TXTSIZ*4];
  fprintf_ftype old_fprintf;
  void *old_stream;
  char *p;

  switch (suffixtext[2])
    {
    case 'l': /* SIL or LIL */
      buf->nn_len = 3;
      break;
    case 's': /* SIS or LIS */
      buf->nn_len = 2;
      break;
    default:
      abort ();
    }
  if (!fetch_data (buf, info, 1)
      || buf->data[1] == 0x40
      || buf->data[1] == 0x49
      || buf->data[1] == 0x52
      || buf->data[1] == 0x5b)
    {
      /* Double prefix, or end of data.  */
      info->fprintf_func (info->stream, ".db 0x%02x ; %s", (unsigned)buf->data[0], suffixtext);
      buf->n_used = 1;
      return buf->n_used;
    }

  old_fprintf = info->fprintf_func;
  old_stream = info->stream;
  info->fprintf_func = (fprintf_ftype) &sprintf;
  info->stream = mybuf;
  mybuf[0] = 0;
  buf->base++;
  if (print_insn_z80_buf (buf, info) >= 0)
    buf->n_used++;
  info->fprintf_func = old_fprintf;
  info->stream = old_stream;

  for (p = mybuf; *p; ++p)
    if (*p == ' ')
      break;
  if (*p)
    {
      *p++ = '\0';
      info->fprintf_func (info->stream, "%s.%s %s", mybuf, suffixtext, p);
    }
  else
    info->fprintf_func (info->stream, "%s.%s", mybuf, suffixtext);
  return buf->n_used;
}