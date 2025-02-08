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