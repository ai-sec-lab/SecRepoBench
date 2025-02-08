void ProcessRow(const RowInfo& input_rows, const RowInfo& outputrowinfo,
                  size_t xextra, size_t xsize, size_t xpos, size_t ypos,
                  float* JXL_RESTRICT temp) const final {
    PROFILER_ZONE("Blend");
    JXL_ASSERT(initialized_);
    const FrameOrigin& frame_origin = state_.frame_header.frame_origin;
    ssize_t bg_xpos = frame_origin.x0 + static_cast<ssize_t>(xpos);
    ssize_t bg_ypos = frame_origin.y0 + static_cast<ssize_t>(ypos);
    int offset = 0;
    if (bg_xpos + static_cast<ssize_t>(xsize) <= 0 ||
        frame_origin.x0 >= static_cast<ssize_t>(image_xsize_) || bg_ypos < 0 ||
        bg_ypos >= static_cast<ssize_t>(image_ysize_)) {
      return;
    }
    if (bg_xpos < 0) {
      offset -= bg_xpos;
      xsize += bg_xpos;
      bg_xpos = 0;
    }
    if (bg_xpos + xsize > image_xsize_) {
      xsize =
          std::max<ssize_t>(0, static_cast<ssize_t>(image_xsize_) - bg_xpos);
    }
    std::vector<const float*> bg_row_ptrs_(input_rows.size());
    std::vector<float*> fg_row_ptrs_(input_rows.size());
    // <MASK>
  }