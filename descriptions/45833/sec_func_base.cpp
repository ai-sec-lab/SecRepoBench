void ProcessRow(const RowInfo& input_rows, const RowInfo& output_rows,
                  size_t xextra, size_t xsize, size_t xpos, size_t ypos,
                  float* JXL_RESTRICT temp) const final {
    if (ypos >= height_) return;
    const float* line_buffers[4];
    for (size_t c = 0; c < 3; c++) {
      line_buffers[c] = GetInputRow(input_rows, c, 0) - xextra;
    }
    if (has_alpha_) {
      line_buffers[3] = GetInputRow(input_rows, alpha_c_, 0) - xextra;
    } else {
      // No xextra offset; opaque_alpha_ is a way to set all values to 1.0f.
      line_buffers[3] = opaque_alpha_.data();
    }
    // TODO(veluca): SIMD.
    ssize_t limit = std::min(xextra + xsize, width_ - xpos);
    for (ssize_t x0 = -xextra; x0 < limit; x0 += kMaxPixelsPerCall) {
      size_t j = 0;
      size_t ix = 0;
      for (; ix < kMaxPixelsPerCall && ssize_t(ix) + x0 < limit; ix++) {
        temp[j++] = line_buffers[0][ix];
        temp[j++] = line_buffers[1][ix];
        temp[j++] = line_buffers[2][ix];
        if (rgba_) {
          temp[j++] = line_buffers[3][ix];
        }
      }
      pixel_callback_(temp, xpos + x0, ypos, ix);
      for (size_t c = 0; c < 3; c++) line_buffers[c] += kMaxPixelsPerCall;
      if (has_alpha_) line_buffers[3] += kMaxPixelsPerCall;
    }
  }