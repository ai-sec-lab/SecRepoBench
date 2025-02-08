void LJpegDecompressor::decodeScan()
{
  if (predictorMode != 1)
    ThrowRDE("Unsupported predictor mode: %u", predictorMode);

  for (uint32 i = 0; i < frame.cps;  i++)
    if (frame.compInfo[i].superH != 1 || frame.compInfo[i].superV != 1)
      ThrowRDE("Unsupported subsampling");

  assert(static_cast<unsigned>(mRaw->dim.x) > offX);
  if ((mRaw->getCpp() * (mRaw->dim.x - offX)) < frame.cps)
    ThrowRDE("Got less pixels than the components per sample");

  // <MASK>
}