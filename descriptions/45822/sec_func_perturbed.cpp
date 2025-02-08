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
    size_t num_c = std::min(input_rows.size(), extra_channel_info_->size() + 3);
    for (size_t c = 0; c < num_c; ++c) {
      fg_row_ptrs_[c] = GetInputRow(input_rows, c, 0) + offset;
      if (c < 3) {
        bg_row_ptrs_[c] =
            bg_->xsize() != 0 && bg_->ysize() != 0
                ? bg_->color()->ConstPlaneRow(c, bg_ypos) + bg_xpos
                : zeroes_.data();
      } else {
        const ImageBundle& ec_bg =
            *state_
                 .reference_frames[state_.frame_header
                                       .extra_channel_blending_info[c - 3]
                                       .source]
                 .frame;
        bg_row_ptrs_[c] =
            ec_bg.xsize() != 0 && ec_bg.ysize() != 0
                ? ec_bg.extra_channels()[c - 3].ConstRow(bg_ypos) + bg_xpos
                : zeroes_.data();
      }
    }
    PerformBlending(bg_row_ptrs_.data(), fg_row_ptrs_.data(),
                    fg_row_ptrs_.data(), 0, xsize, blending_info_[0],
                    blending_info_.data() + 1, *extra_channel_info_);
  }