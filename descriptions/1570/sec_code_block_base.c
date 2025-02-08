int audio_channel = 0;
        INTFLOAT gain;
        if (s->channel_mode == AC3_CHMODE_DUALMONO && ch <= 2)
            audio_channel = 2-ch;
        if (s->heavy_compression && s->compression_exists[audio_channel])
            gain = s->heavy_dynamic_range[audio_channel];
        else
            gain = s->dynamic_range[audio_channel];

#if USE_FIXED
        scale_coefs(s->transform_coeffs[ch], s->fixed_coeffs[ch], gain, 256);
#else
        if (s->target_level != 0)
          gain = gain * s->level_gain[audio_channel];
        gain *= 1.0 / 4194304.0f;
        s->fmt_conv.int32_to_float_fmul_scalar(s->transform_coeffs[ch],
                                               s->fixed_coeffs[ch], gain, 256);
#endif