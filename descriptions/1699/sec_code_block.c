
        av_log(m->avctx, AV_LOG_ERROR,
               "Number of primitive matrices cannot be greater than %d.\n",
               max_primitive_matrices);
        s->num_primitive_matrices = 0;
        return AVERROR_INVALIDDATA;
    