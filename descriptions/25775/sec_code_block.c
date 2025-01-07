
        unsigned chunk = AV_RL32(p->buf + i);
        unsigned size  = AV_RB32(p->buf + i + 4);

        if (size < 8 || p->buf_size - i < size)
            return 0;
        i += 8;
        size -= 8;
        switch (chunk) {
        case MKTAG('C','T','R','L'):
            break;
        case MKTAG('S','N','D','S'):
            if (size < 56)
                return 0;
            i += 8;
            if (AV_RL32(p->buf + i) != MKTAG('S','H','D','R'))
                return 0;
            i += 28;

            if (AV_RB32(p->buf + i) <= 0)
                return 0;
            i += 4;
            if (AV_RB32(p->buf + i) <= 0)
                return 0;
            i += 4;
            if (AV_RL32(p->buf + i) == MKTAG('S','D','X','2'))
                return AVPROBE_SCORE_MAX;
            else
                return 0;
            break;
        case MKTAG('S','H','D','R'):
            if (size > 0x78) {
                i += 0x78;
                size -= 0x78;
            }
            break;
        default:
            break;
        }

        i += size;
    