if (!sample_size)
            return AVERROR_INVALIDDATA;
        dts += sample_duration;
        offset += sample_size;
        sc->data_size += sample_size;

        if (sample_duration <= INT64_MAX - sc->duration_for_fps &&
            1 <= INT_MAX - sc->nb_frames_for_fps
        ) {
            sc->duration_for_fps += sample_duration;
            sc->nb_frames_for_fps ++;
        }