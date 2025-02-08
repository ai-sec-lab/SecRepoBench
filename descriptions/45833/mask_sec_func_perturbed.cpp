void ProcessRow(const RowInfo& inrows, const RowInfo& output_rows,
                  size_t xextra, size_t xsize, size_t xpos, size_t ypos,
                  float* JXL_RESTRICT temp) const final {
    if (ypos >= height_) return;
    const float* line_buffers[4];
    // <MASK>
  }