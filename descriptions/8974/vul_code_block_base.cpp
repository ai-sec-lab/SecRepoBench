oSrc.bytes_in_buffer = (size_t)nSizeIn;
    oSrc.term_source = RMFJPEGNoop;
    oSrc.init_source = RMFJPEGNoop;

    jpeg_create_decompress(&oJpegInfo);

    memset(&oJmpBuf, 0, sizeof(jmp_buf));