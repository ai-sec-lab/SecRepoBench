static inline void
dxf_skip_ws (Bit_Chain *dat)
{
  const int is_binary = dat->opts & DWG_OPTS_DXFB;
  if (is_binary)
    return;
  if (dat->byte >= dat->size)
    return;
  // clang-format off
  for (; (!dat->chain[dat->byte] ||
          dat->chain[dat->byte] == ' ' ||
          dat->chain[dat->byte] == '\t' ||
          dat->chain[dat->byte] == '\r');
       )
    // clang-format on
    {
      dat->byte++;
      if (dat->byte >= dat->size)
        return;
    }
}