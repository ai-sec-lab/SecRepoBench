static int
suffix (struct buffer *buf, disassemble_info *info, const char *txt)
{
  char mybuf[TXTSIZ*4];
  fprintf_ftype old_fprintf;
  void *old_stream;
  char *p;

  switch (txt[2])
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
      info->fprintf_func (info->stream, ".db 0x%02x ; %s", (unsigned)buf->data[0], txt);
      buf->n_used = 1;
      return buf->n_used;
    }

  old_fprintf = info->fprintf_func;
  // <MASK>
  info->fprintf_func = old_fprintf;
  info->stream = old_stream;

  for (p = mybuf; *p; ++p)
    if (*p == ' ')
      break;
  if (*p)
    {
      *p++ = '\0';
      info->fprintf_func (info->stream, "%s.%s %s", mybuf, txt, p);
    }
  else
    info->fprintf_func (info->stream, "%s.%s", mybuf, txt);
  return buf->n_used;
}