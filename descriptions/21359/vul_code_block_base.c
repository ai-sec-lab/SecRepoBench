old_stream = info->stream;
  info->fprintf_func = (fprintf_ftype) &sprintf;
  info->stream = mybuf;
  buf->base++;
  if (print_insn_z80_buf (buf, info) >= 0)
    buf->n_used++;