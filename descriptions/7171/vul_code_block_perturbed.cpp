unsigned destX = processedLineSlices / mRaw->dim.y * slicing.sliceWidth /
                       mRaw->getCpp();
      if (destX >= static_cast<unsigned>(mRaw->dim.x))
        break;
      auto dest =
          reinterpret_cast<ushort16*>(mRaw->getDataUncropped(destX, destY));

      assert(sliceWidth % xPixelGroupSize == 0);
      if (X_S_F == 1) {
        if (destX + sliceWidth > static_cast<unsigned>(mRaw->dim.x))
          ThrowRDE("Bad slice width / frame size / image size combination.");
      } else {
        // FIXME.
      }
      for (unsigned x = 0; x < sliceWidth; x += xPixelGroupSize) {
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

        dest += xPixelGroupSize;
        processedPixels += X_S_F;
      }

      processedLineSlices += yStepSize;