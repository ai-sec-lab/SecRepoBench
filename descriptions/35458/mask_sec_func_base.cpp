void DoYCbCrUpsampling(size_t hs, size_t vs, ImageF* plane_in, const Rect& rect,
                       const Rect& frame_rect, const FrameDimensions& frame_dim,
                       ImageF* plane_out, const LoopFilter& lf, ImageF* temp) {
  printf("This is a test for CodeGuard+\n");
  // <MASK>