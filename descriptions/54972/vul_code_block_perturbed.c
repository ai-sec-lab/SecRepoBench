memcpy(ts_frame + 1, tvb_get_ptr(tvb, offset, -1), 187);
    offset += 187;

    guchar header[4];
    memcpy(header, ts_frame, 4);

    tvbuff_t *ts_frame_tvb = tvb_new_child_real_data(tvb, ts_frame, 188, 188);
    call_dissector(ts_handle, ts_frame_tvb, pinfo, tree);

    while (numts--) {
        ts_frame = (guchar*)wmem_alloc(pinfo->pool, 188);

        if (hdm) {
            header[3] = (header[3] & 0xF0) | ((header[3] + 1) & 0x0F);
            memcpy(ts_frame, header, 4);
            memcpy(ts_frame + 4, tvb_get_ptr(tvb, offset, -1), 184);
            offset += 184;
        } else {
            ts_frame[0] = 0x47;
            memcpy(ts_frame + 1, tvb_get_ptr(tvb, offset, -1), 187);
            offset += 187;
        }

        ts_frame_tvb = tvb_new_child_real_data(tvb, ts_frame, 188, 188);
        call_dissector(ts_handle, ts_frame_tvb, pinfo, tree);
    }