void LJpegDecompressor::decodeN()
{
  assert(mRaw->getCpp() > 0);
  assert(N_COMP > 0);
  assert(N_COMP >= mRaw->getCpp());
  assert((N_COMP / mRaw->getCpp()) > 0);

  assert(mRaw->dim.x >= N_COMP);
  assert((mRaw->getCpp() * (mRaw->dim.x - offX)) >= N_COMP);

  auto ht = getHuffmanTables<N_COMP>();
  auto pred = getInitialPredictors<N_COMP>();
  auto predNext = pred.data();

  BitPumpJPEG bitStream(input);

  for (unsigned y = 0; y < frame.h; ++y) {
    auto destinationY = offY + y;
    // A recoded DNG might be split up into tiles of self contained LJpeg
    // blobs. The tiles at the bottom and the right may extend beyond the
    // dimension of the raw image buffer. The excessive content has to be
    // ignored. For y, we can simply stop decoding when we reached the border.
    if (destinationY >= static_cast<unsigned>(mRaw->dim.y))
      break;

    auto dest =
        reinterpret_cast<ushort16*>(mRaw->getDataUncropped(offX, destinationY));

    copy_n(predNext, N_COMP, pred.data());
    // the predictor for the next line is the start of this line
    predNext = dest;

    unsigned width =
        std::min(frame.w, (mRaw->getCpp() * (mRaw->dim.x - offX)) / N_COMP);

    // For x, we first process all pixels within the image buffer ...
    for (unsigned x = 0; x < width; ++x) {
      unroll_loop<N_COMP>([&](int i) {
        *dest++ = pred[i] += ht[i]->decodeNext(bitStream);
      });
    }
    // ... and discard the rest.
    for (unsigned x = width; x < frame.w; ++x) {
      unroll_loop<N_COMP>([&](int i) {
        ht[i]->decodeNext(bitStream);
      });
    }
  }
}