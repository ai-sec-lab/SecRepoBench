Status PredictICC(const uint8_t* icc, size_t size, PaddedBytes* result) {
  PaddedBytes commands;
  PaddedBytes data;

  // <MASK>

  EncodeVarInt(commands.size(), result);
  for (size_t i = 0; i < commands.size(); i++) {
    result->push_back(commands[i]);
  }
  for (size_t i = 0; i < data.size(); i++) {
    result->push_back(data[i]);
  }

  return true;
}