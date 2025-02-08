static inline void
dxf_skip_ws (Bit_Chain *dat)
{
  const int is_binary = dat->opts & DWG_OPTS_DXFB;
  if (is_binary)
    return;
  // <MASK>
}