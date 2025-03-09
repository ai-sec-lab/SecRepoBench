if (s->num_primitive_matrices > maxallowedmatrices) {
        av_log(m->avctx, AV_LOG_ERROR,
               "Number of primitive matrices cannot be greater than %d.\n",
               maxallowedmatrices);
        s->num_primitive_matrices = 0;
        return AVERROR_INVALIDDATA;
    }