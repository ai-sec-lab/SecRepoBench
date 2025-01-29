DEBUG_MSG (ARABIC, buffer, "Shifting %d's: %d %d\n", cc, currentindex, j);
    hb_glyph_info_t temp[HB_OT_SHAPE_COMPLEX_MAX_COMBINING_MARKS];
    assert (j - currentindex <= ARRAY_LENGTH (temp));
    buffer->merge_out_clusters (start, j);
    memmove (temp, &info[currentindex], (j - currentindex) * sizeof (hb_glyph_info_t));
    memmove (&info[start + j - currentindex], &info[start], (currentindex - start) * sizeof (hb_glyph_info_t));
    memmove (&info[start], temp, (j - currentindex) * sizeof (hb_glyph_info_t));