buf[1] = base;
    if (neg) buf[1] = 0x80;
    memcpy(buf+2, p, strlen);