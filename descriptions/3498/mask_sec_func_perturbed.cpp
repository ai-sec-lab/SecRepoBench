void SamsungV2Decompressor::decompressRow(uint32 line) {
  // The format is relatively straightforward. Each line gets encoded as a set
  // of differences from pixels from another line. Pixels are grouped in blocks
  // of 16 (8 green, 8 red or blue). Each block is encoded in three sections.
  // First 1 or 4 bits to specify which reference pixels to use, then a section
  // that specifies for each pixel the number of bits in the difference, then
  // the actual difference bits

  // Align pump to 16byte boundary
  const auto line_offset = data.getPosition();
  if ((line_offset & 0xf) != 0)
    data.skipBytes(16 - (line_offset & 0xf));

  BitPumpMSB32 pump(data);

  auto* img = reinterpret_cast<ushort16*>(mRaw->getData(0, line));
  ushort16* img_up = reinterpret_cast<ushort16*>(
      mRaw->getData(0, std::max(0, static_cast<int>(line) - 1)));
  ushort16* img_up2 = reinterpret_cast<ushort16*>(
      mRaw->getData(0, std::max(0, static_cast<int>(line) - 2)));

  // Initialize the motion and diff modes at the start of the line
  uint32 motion = 7;
  // By default we are not scaling values at all
  int32 scale = 0;

  uint32 diffBitsMode[3][2] = {{0}};
  for (auto& i : diffBitsMode)
    i[0] = i[1] = (line == 0 || line == 1) ? 7 : 4;

  assert(width >= 16);
  // <MASK>
}