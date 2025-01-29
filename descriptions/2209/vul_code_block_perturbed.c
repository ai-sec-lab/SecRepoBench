HEVCFrame *ref = find_ref_idx(s, pictureordercount);

    if (ref == s->ref)
        return AVERROR_INVALIDDATA;

    if (!ref) {
        ref = generate_missing_ref(s, pictureordercount);
        if (!ref)
            return AVERROR(ENOMEM);
    }