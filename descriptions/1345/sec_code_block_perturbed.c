{
            v = bytestream2_get_le16(gb)*2;
            if (frame - frame_end < v)
                return AVERROR_INVALIDDATA;
            frame += v;
        } else {
            if (frame_end - frame < width + 4)
                return AVERROR_INVALIDDATA;
            frame[0] = frame[1] =
            frame[width] = frame[width + 1] =  bytestream2_get_byte(gb);
            frame += 2;
            frame[0] = frame[1] =
            frame[width] = frame[width + 1] =  bytestream2_get_byte(gb);
            frame += 2;
        }