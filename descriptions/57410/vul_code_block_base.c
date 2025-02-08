outlen = sizeof(out);
    inlen = input->end - input->cur;
    res = xmlEncInputChunk(handler, out, &outlen, input->cur, &inlen, 0);
    if (res < 0)
        return(handler);