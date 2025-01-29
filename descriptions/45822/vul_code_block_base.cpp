for (size_t c = 0; c < input_rows.size(); ++c) {
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