if (!memcmp(data, "OpusHead", sizeof(char)*8))
        return 0;
    if (!memcmp(data, "OpusTags", sizeof(char)*8))
        return 0;

    GF_LOG(GF_LOG_DEBUG, GF_LOG_CODING, ("Processing Opus packet, self: %d, size %d\n", self_delimited, data_length));

    if (data_length < 1) {
        GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Opus packet size must be at least one to parse TOC byte\n"));
        return 0;
    }
    memset(pktheader, 0, sizeof(GF_OpusPacketHeader));
    pktheader->self_delimited = self_delimited;
    pktheader->TOC_config = (data[0] & 0xf8) >> 3;
    pktheader->TOC_stereo = (data[0] & 0x4) >> 2;
    pktheader->TOC_code = data[0] & 0x03;
    pktheader->size = 1;
    if (pktheader->TOC_code == 0) {
        pktheader->nb_frames = 1;
        if (self_delimited) {
            nb_read_bytes = gf_opus_read_length(data, data_length, pktheader->size, &pktheader->self_delimited_length);
            if (nb_read_bytes) {
                pktheader->size += nb_read_bytes;
            } else {
                GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Could not read self delimited length in Opus packet code 0\n"));
                return 0;
            }
//            0                   1                   2                   3
//            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           | config  |s|0|0| N1 (1-2 bytes):                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
//           |               Compressed frame 1 (N1 bytes)...                :
//           :                                                               |
//           |                                                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            pktheader->frame_lengths[0] = pktheader->self_delimited_length;
        } else {
//            0                   1                   2                   3
//            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           | config  |s|0|0|                                               |
//           +-+-+-+-+-+-+-+-+                                               |
//           |                    Compressed frame 1 (N-1 bytes)...          :
//           :                                                               |
//           |                                                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            pktheader->frame_lengths[0] = data_length - pktheader->size;
        }
        pktheader->packet_size = pktheader->size + pktheader->frame_lengths[0];
    } else if (pktheader->TOC_code == 1) {
        pktheader->nb_frames = 2;
        if (self_delimited) {
            nb_read_bytes = gf_opus_read_length(data, data_length, pktheader->size, &pktheader->self_delimited_length);
            if (nb_read_bytes) {
                pktheader->size += nb_read_bytes;
            } else {
                GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Could not read self delimited length in Opus packet code 1\n"));
                return 0;
            }
//            0                   1                   2                   3
//            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           | config  |s|0|1| N1 (1-2 bytes):                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               :
//           |               Compressed frame 1 (N1 bytes)...                |
//           :                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           |                               |                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               :
//           |               Compressed frame 2 (N1 bytes)...                |
//           :                                               +-+-+-+-+-+-+-+-+
//           |                                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            pktheader->frame_lengths[0] = pktheader->self_delimited_length;
            pktheader->frame_lengths[1] = pktheader->self_delimited_length;
        } else {
//            0                   1                   2                   3
//            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           | config  |s|0|1|                                               |
//           +-+-+-+-+-+-+-+-+                                               :
//           |             Compressed frame 1 ((N-1)/2 bytes)...             |
//           :                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           |                               |                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               :
//           |             Compressed frame 2 ((N-1)/2 bytes)...             |
//           :                                               +-+-+-+-+-+-+-+-+
//           |                                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            if ((data_length-pktheader->size) % 2) {
                GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Size of non-self-delimited Opus packet with code 2 must be even but is %d\n",data_length-pktheader->size));
                return 0;
            }
            pktheader->frame_lengths[0] = (data_length-pktheader->size)/2;
            pktheader->frame_lengths[1] = (data_length-pktheader->size)/2;
        }
        pktheader->packet_size = pktheader->size + pktheader->frame_lengths[0] + pktheader->frame_lengths[1];
    } else if (pktheader->TOC_code == 2) {
        pktheader->nb_frames = 2;
        if (self_delimited) {
            nb_read_bytes = gf_opus_read_length(data, data_length, pktheader->size, &pktheader->self_delimited_length);
            if (nb_read_bytes) {
                pktheader->size += nb_read_bytes;
            } else {
                GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Could not read self delimited length in Opus packet code 2\n"));
                return 0;
            }
        }
        nb_read_bytes = gf_opus_read_length(data, data_length, pktheader->size, &pktheader->code2_frame_length);
        if (nb_read_bytes) {
            pktheader->size += nb_read_bytes;
        } else {
            GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Could not read frame length in Opus packet code 2\n"));
            return 0;
        }
        if (self_delimited) {
//            0                   1                   2                   3
//            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           | config  |s|1|0| N1 (1-2 bytes): N2 (1-2 bytes :               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+               :
//           |               Compressed frame 1 (N1 bytes)...                |
//           :                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           |                               |                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
//           |               Compressed frame 2 (N2 bytes)...                :
//           :                                                               |
//           |                                                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            pktheader->frame_lengths[0] = pktheader->self_delimited_length;
            pktheader->frame_lengths[1] = pktheader->code2_frame_length;
        } else {
//            0                   1                   2                   3
//            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           | config  |s|1|0| N1 (1-2 bytes):                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               :
//           |               Compressed frame 1 (N1 bytes)...                |
//           :                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//           |                               |                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
//           |                     Compressed frame 2...                     :
//           :                                                               |
//           |                                                               |
//           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            pktheader->frame_lengths[0] = pktheader->code2_frame_length;
            pktheader->frame_lengths[1] = data_length - pktheader->size - pktheader->code2_frame_length;
        }
        pktheader->packet_size = pktheader->size + pktheader->frame_lengths[0] + pktheader->frame_lengths[1];
    } else if (pktheader->TOC_code == 3) {
        u32 sum = 0;
        if (data_length <= pktheader->size) {
            GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Not enough data to parse TOC code 3 data\n"));
            return 0;
        }
        pktheader->code3_vbr = (data[pktheader->size] & 0x80) >> 7;
        pktheader->code3_padding = (data[pktheader->size] & 0x40) >> 6;
        pktheader->nb_frames = data[pktheader->size] & 0x3f;
        pktheader->size++;
        if (pktheader->code3_padding) {
            if (data_length <= pktheader->size) {
                GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Not enough data to parse TOC code 3 padding length\n"));
                return 0;
            }
            if (data[pktheader->size] == 255) {
                pktheader->code3_padding_length = 254 + data[pktheader->size+1];
                pktheader->size += 2;
            } else {
                pktheader->code3_padding_length = data[pktheader->size];
                pktheader->size++;
            }
        } else {
            pktheader->code3_padding_length = 0;
        }
        if (self_delimited) {
            nb_read_bytes = gf_opus_read_length(data, data_length, pktheader->size, &pktheader->self_delimited_length);
            if (nb_read_bytes) {
                pktheader->size += nb_read_bytes;
            } else {
                GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Could not read self delimited length in Opus packet code 3\n"));
                return 0;
            }
        }
        if (pktheader->code3_vbr) {
            u32 max;
            u32 min;
            if (self_delimited) {
//                0                   1                   2                   3
//                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               | config  |s|1|1|1|p|     M     | Padding length (Optional)     :
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               : N1 (1-2 bytes):     ...       :     N[M-1]    |     N[M]      :
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 1 (N1 bytes)...                :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 2 (N2 bytes)...                :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :                              ...                              :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :              Compressed frame M (N[M] bytes)...               :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               :                  Opus Padding (Optional)...                   |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                pktheader->frame_lengths[0] = pktheader->self_delimited_length;
                min = 1;
                max = pktheader->nb_frames;
                sum += pktheader->frame_lengths[0];
            } else {
//                0                   1                   2                   3
//                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               | config  |s|1|1|1|p|     M     | Padding length (Optional)     :
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               : N1 (1-2 bytes): N2 (1-2 bytes):     ...       :     N[M-1]    |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 1 (N1 bytes)...                :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 2 (N2 bytes)...                :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :                              ...                              :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :                     Compressed frame M...                     :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               :                  Opus Padding (Optional)...                   |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                min = 0;
                max = pktheader->nb_frames-1;
            }
            for (i = min; i < max; i++) {
                if (data_length <= pktheader->size) {
                    GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Not enough data to parse TOC code 3 length\n"));
                    return 0;
                }
                nb_read_bytes = gf_opus_read_length(data, data_length, pktheader->size, &(pktheader->frame_lengths[i]));
                if (nb_read_bytes) {
                    pktheader->size += nb_read_bytes;
                } else {
                    GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Could not read frame length in Opus packet code 3\n"));
                    return 0;
                }
                sum += pktheader->frame_lengths[i];
            }
            if (!self_delimited) {
                pktheader->frame_lengths[pktheader->nb_frames-1] = data_length - pktheader->size - pktheader->code3_padding_length - sum;
                sum += pktheader->frame_lengths[pktheader->nb_frames-1];
            }
        } else {
            u32 cbr_length;
            if (self_delimited) {
//                0                   1                   2                   3
//                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               | config  |s|1|1|0|p|     M     | Pad len (Opt) : N1 (1-2 bytes):
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 1 (N1 bytes)...                :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 2 (N1 bytes)...                :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :                              ...                              :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame M (N1 bytes)...                :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               :                  Opus Padding (Optional)...                   |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                cbr_length = pktheader->self_delimited_length;
            } else {
//                0                   1                   2                   3
//                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               | config  |s|1|1|0|p|     M     |  Padding length (Optional)    :
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 1 (R/M bytes)...               :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame 2 (R/M bytes)...               :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :                              ...                              :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               |                                                               |
//               :               Compressed frame M (R/M bytes)...               :
//               |                                                               |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//               :                  Opus Padding (Optional)...                   |
//               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                if ((data_length - pktheader->size - pktheader->code3_padding_length) % pktheader->nb_frames) {
                    GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("Sum of frame lengths is not a multiple of the number of frames\n"));
                    return 0;
                }
                cbr_length = (data_length - pktheader->size - pktheader->code3_padding_length)/pktheader->nb_frames;
            }
            for (i = 0; i < pktheader->nb_frames; i++) {
                pktheader->frame_lengths[i] = cbr_length;
                sum += pktheader->frame_lengths[i];
            }
        }
        pktheader->packet_size = pktheader->size + pktheader->code3_padding_length + sum;
    }
    return 1;