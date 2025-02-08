void LJpegDecompressor::decodeScan()
{
  if (predictorMode != 1)
    ThrowRDE("Unsupported predictor mode: %u", predictorMode);

  for (uint32 componentIndex = 0; componentIndex < frame.cps;  componentIndex++)
    if (frame.compInfo[componentIndex].superH != 1 || frame.compInfo[componentIndex].superV != 1)
      ThrowRDE("Unsupported subsampling");

  assert(static_cast<unsigned>(mRaw->dim.x) > offX);
  if ((mRaw->getCpp() * (mRaw->dim.x - offX)) < frame.cps)
    ThrowRDE("Got less pixels than the components per sample");

  // <MASK>
}