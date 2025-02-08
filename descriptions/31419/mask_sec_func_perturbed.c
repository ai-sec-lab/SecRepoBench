EXPORT int
dwg_read_dxf (Bit_Chain *restrict dat, Dwg_Data *restrict drawing)
{
  // const int minimal = dwg->opts & DWG_OPTS_MINIMAL;
  Dxf_Pair *pair = NULL;
  int error = 0;

  loglevel = drawing->opts & DWG_OPTS_LOGLEVEL;
  if (!dat->chain && dat->fh)
    {
      error = dat_read_stream (dat, dat->fh);
      if (error >= DWG_ERR_CRITICAL)
        return error;
      if (dat->size >= 22 &&
          !memcmp (dat->chain, "AutoCAD Binary DXF",
                   sizeof ("AutoCAD Binary DXF") - 1))
        {
          dat->opts |= DWG_OPTS_DXFB;
          dat->byte = 22;
        }
    }
  if (dat->size < 256)
    {
      LOG_ERROR ("DXF input too small, %lu byte.\n", dat->size);
      return DWG_ERR_IOERROR;
    }
  /* Fail early on DWG */
  if (!memcmp (dat->chain, "AC10", 4) ||
      !memcmp (dat->chain, "AC1.", 4) ||
      !memcmp (dat->chain, "AC2.10", 4) ||
      !memcmp (dat->chain, "MC0.0", 4))
    {
      LOG_ERROR ("This is a DWG, not a DXF\n");
      return DWG_ERR_INVALIDDWG;
    }
  /* Fuzzers like to skip zero-termination. And fail loudly on strtol */
  // <MASK>
}