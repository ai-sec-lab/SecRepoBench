DEBUG_MSG (ARABIC, buffer, "Shifting %d's: %d %d\n", cc, i, j);
    hb_glyph_info_t temp[HB_OT_SHAPE_COMPLEX_MAX_COMBINING_MARKS];
    assert (j - i <= ARRAY_LENGTH (temp));
    buffer->merge_clusters (start, j);
    memmove (temp, &info[i], (j - i) * sizeof (hb_glyph_info_t));
    memmove (&info[start + j - i], &info[start], (i - start) * sizeof (hb_glyph_info_t));
    memmove (&info[start], temp, (j - i) * sizeof (hb_glyph_info_t));