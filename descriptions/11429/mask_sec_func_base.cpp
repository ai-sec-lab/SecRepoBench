void VC5Decompressor::Wavelet::HighPassBand::decode(const Wavelet& wavelet) {
  auto dequantize = [quant = quant](int16_t val) -> int16_t {
    return mVC5DecompandingTable[uint16_t(val)] * quant;
  };

  data = Array2DRef<int16_t>::create(wavelet.width, wavelet.height);
  const Array2DRef<int16_t> dst(data.data(), wavelet.width, wavelet.height);

  BitPumpMSB bits(bs);
  // decode highpass band
  int pixelValue = 0;
  unsigned int count = 0;
  int nPixels = wavelet.width * wavelet.height;
  for (int iPixel = 0; iPixel < nPixels;) {
    getRLV(&bits, &pixelValue, &count);
    // <MASK>
  }
  if (bits.getPosition() < bits.getSize()) {
    getRLV(&bits, &pixelValue, &count);
    if (pixelValue != MARKER_BAND_END || count != 0)
      ThrowRDE("EndOfBand marker not found");
  }
}