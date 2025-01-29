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