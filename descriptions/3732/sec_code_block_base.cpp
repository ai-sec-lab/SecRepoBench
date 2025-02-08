if (frame.h * std::accumulate(slicesWidths.begin(), slicesWidths.end(), 0) <
      mRaw->dim.area())
    ThrowRDE("Incorrrect slice height / slice widths! Less than image size.");

  // To understand the CR2 slice handling and sampling factor behavior, see
  // https://github.com/lclevy/libcraw2/blob/master/docs/cr2_lossless.pdf?raw=true

  // inner loop decodes one group of pixels at a time
  //  * for <N,1,1>: N  = N*1*1 (full raw)
  //  * for <3,2,1>: 6  = 3*2*1
  //  * for <3,2,2>: 12 = 3*2*2
  // and advances x by N_COMP*X_S_F and y by Y_S_F
  constexpr int xStepSize = N_COMP * X_S_F;
  constexpr int yStepSize = Y_S_F;

  unsigned processedPixels = 0;
  unsigned processedLineSlices = 0;
  for (unsigned sliceWidth : slicesWidths) {
    for (unsigned y = 0; y < frame.h; y += yStepSize) {
      // Fix for Canon 80D mraw format.
      // In that format, `frame` is 4032x3402, while `mRaw` is 4536x3024.
      // Consequently, the slices in `frame` wrap around plus there are few
      // 'extra' sliced lines because sum(slicesW) * sliceH > mRaw->dim.area()
      // Those would overflow, hence the break.
      // see FIX_CANON_FRAME_VS_IMAGE_SIZE_MISMATCH
      unsigned destY = processedLineSlices % mRaw->dim.y;
      unsigned destX =
          processedLineSlices / mRaw->dim.y * slicesWidths[0] / mRaw->getCpp();
      if (destX >= static_cast<unsigned>(mRaw->dim.x))
        break;
      auto dest =
          reinterpret_cast<ushort16*>(mRaw->getDataUncropped(destX, destY));

      for (unsigned x = 0; x < sliceWidth; x += xStepSize) {
        // check if we processed one full raw row worth of pixels
        if (processedPixels == frame.w) {
          // if yes -> update predictor by going back exactly one row,
          // no matter where we are right now.
          // makes no sense from an image compression point of view, ask Canon.
          copy_n(predNext, N_COMP, pred.data());
          predNext = dest;
          processedPixels = 0;
        }

        if (X_S_F == 1) { // will be optimized out
          unroll_loop<N_COMP>([&](int i) {
            dest[i] = pred[i] += ht[i]->decodeNext(bitStream);
          });
        } else {
          unroll_loop<Y_S_F>([&](int i) {
            dest[0 + i*pixelPitch] = pred[0] += ht[0]->decodeNext(bitStream);
            dest[3 + i*pixelPitch] = pred[0] += ht[0]->decodeNext(bitStream);
          });

          dest[1] = pred[1] += ht[1]->decodeNext(bitStream);
          dest[2] = pred[2] += ht[2]->decodeNext(bitStream);
        }

        dest += xStepSize;
        processedPixels += X_S_F;
      }

      processedLineSlices += yStepSize;
    }
  }