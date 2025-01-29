outlen = sizeof(out);
    inlen = input->end - input->cur;
    result = xmlEncInputChunk(handler, out, &outlen, input->cur, &inlen, 0);
    if (result < 0)
        return(handler);