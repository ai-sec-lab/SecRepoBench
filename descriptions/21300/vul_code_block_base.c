outf->bytes_per_chunk = 4;

  (*outf->read_memory_func) (pc & ~0x03, buf, 4, outf);