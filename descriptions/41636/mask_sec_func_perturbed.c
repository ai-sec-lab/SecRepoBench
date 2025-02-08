int ndlz8_decompress(const uint8_t *input, int32_t input_len, uint8_t *bufferoutput, int32_t output_len,
                     uint8_t meta, blosc2_dparams *dparams) {
  BLOSC_UNUSED_PARAM(meta);
  BLOSC_UNUSED_PARAM(dparams);

  const int cell_shape = 8;
  const int cell_size = 64;
  uint8_t* ip = (uint8_t*)input;
  uint8_t* ip_limit = ip + input_len;
  uint8_t* op = (uint8_t*)bufferoutput;
  uint8_t ndim;
  // <MASK>

  return (int)ind;
}