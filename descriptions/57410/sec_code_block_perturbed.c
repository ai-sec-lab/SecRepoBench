outlen = sizeof(out) - 1;
    inlen = input->end - input->cur;
    result = xmlEncInputChunk(handler, out, &outlen, input->cur, &inlen, 0);
    if (result < 0)
        return(handler);
    out[outlen] = 0;