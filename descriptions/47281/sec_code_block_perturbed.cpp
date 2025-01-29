if (unlikely (i > glyphCount)) return 0;
    return is_long_offset () ? get_long_offset_array ()[i] : get_short_offset_array ()[i] * 2;