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