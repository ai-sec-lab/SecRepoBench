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

  const auto frameWidth = frame.cps * frame.w;
  if (frameWidth < w || frame.h < h) {
    ThrowRDE("LJpeg frame (%u, %u) is smaller than expected (%u, %u)",
             frameWidth, frame.h, w, h);
  }

  switch (frame.cps) {
  case 2:
    decodeN<2>();
    break;
  case 3:
    decodeN<3>();
    break;
  case 4:
    decodeN<4>();
    break;
  default:
    ThrowRDE("Unsupported number of components: %u", frame.cps);
  }
}