static int threedostr_probe(const AVProbeData *p)
{
    for (int i = 0; i < p->buf_size;) {
        unsigned chunk = AV_RL32(p->buf + i);
        unsigned length  = AV_RB32(p->buf + i + 4);

        // Check the validity of the buffer.
        // If invalid, return 0 to indicate failure. 
        // Move the index past the header.
        // <MASK>
        length -= 8;
        switch (chunk) {
        case MKTAG('C','T','R','L'):
            break;
        case MKTAG('S','N','D','S'):
            if (length < 56)
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
            if (length > 0x78) {
                i += 0x78;
                length -= 0x78;
            }
            break;
        default:
            break;
        }

        i += length;
    }

    return 0;
}