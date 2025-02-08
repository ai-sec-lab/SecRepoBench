int status;

  outf->bytes_per_chunk = 4;
  status = (*outf->read_memory_func) (pc & ~0x03, buf, 4, outf);
  if (status)
    {
      (*outf->memory_error_func) (status, pc, outf);
      return -1;
    }