// Do not render padding if group is empty; if group is empty x0, y0 might
    // have arbitrary values (from frame_origin).
    if (group_rect.xsize() > 0 && group_rect.ysize() > 0) {
      if (gx == 0 && gy == 0) {
        RenderPadding(thread_id, Rect(0, 0, x0, y0));
      }
      if (gy == 0) {
        RenderPadding(thread_id, Rect(x0, 0, x1 - x0, y0));
      }
      if (gx == 0) {
        RenderPadding(thread_id, Rect(0, y0, x0, y1 - y0));
      }
      if (gx == 0 && gy + 1 == frame_dimensions_.ysize_groups) {
        RenderPadding(thread_id, Rect(0, y1, x0, full_image_ysize_ - y1));
      }
      if (gy + 1 == frame_dimensions_.ysize_groups) {
        RenderPadding(thread_id, Rect(x0, y1, x1 - x0, full_image_ysize_ - y1));
      }
      if (gy == 0 && gx + 1 == frame_dimensions_.xsize_groups) {
        RenderPadding(thread_id, Rect(x1, 0, full_image_xsize_ - x1, y0));
      }
      if (gx + 1 == frame_dimensions_.xsize_groups) {
        RenderPadding(thread_id, Rect(x1, y0, full_image_xsize_ - x1, y1 - y0));
      }
      if (gy + 1 == frame_dimensions_.ysize_groups &&
          gx + 1 == frame_dimensions_.xsize_groups) {
        RenderPadding(thread_id, Rect(x1, y1, full_image_xsize_ - x1,
                                      full_image_ysize_ - y1));
      }
    }