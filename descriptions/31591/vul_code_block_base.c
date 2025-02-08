const unsigned long endpos = dat->size * 8;
  long bits = (long)dat->bit + advance;
  if (pos + advance > endpos)
    {
      loglevel = dat->opts & DWG_OPTS_LOGLEVEL;
      LOG_ERROR ("%s buffer overflow at pos %lu.%u, size %lu, advance by %ld",
                 __FUNCTION__, dat->byte, dat->bit, dat->size, advance);
    }
  else if ((long)pos + advance < 0)
    {
      loglevel = dat->opts & DWG_OPTS_LOGLEVEL;
      LOG_ERROR ("buffer underflow at pos %lu.%u, size %lu, advance by %ld",
                 dat->byte, dat->bit, dat->size, advance)
      dat->byte = 0;
      dat->bit = 0;
      return;
    }