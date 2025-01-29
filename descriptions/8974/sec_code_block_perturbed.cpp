oSrc.bytes_in_buffer = (size_t)nSizeIn;
    oSrc.term_source = RMFJPEGNoop;
    oSrc.init_source = RMFJPEGNoop;
    oSrc.fill_input_buffer = RMFJPEG_fill_input_buffer_dec;

    jpeg_create_decompress(&oJpegInfo);

    memset(&oJmpBuf, 0, sizeof(jmp_buf));