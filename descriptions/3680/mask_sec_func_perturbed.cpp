void NefDecoder::DecodeNikonSNef(ByteStream* input, uint32 w, uint32 height) {
  if (w < 6)
    ThrowIOE("got a %u wide sNEF, aborting", w);

  // <MASK>
}