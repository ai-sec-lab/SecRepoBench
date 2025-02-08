outlen = sizeof(out) - 1;
    inlen = input->end - input->cur;
    res = xmlEncInputChunk(handler, out, &outlen, input->cur, &inlen, 0);
    if (res < 0)
        return(handler);
    out[outlen] = 0;