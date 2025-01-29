jpegSrc.term_source = RMFJPEGNoop;
    jpegSrc.init_source = RMFJPEGNoop;
    jpegSrc.fill_input_buffer = RMFJPEG_fill_input_buffer_dec;

    jpeg_create_decompress(&oJpegInfo);

    memset(&oJmpBuf, 0, sizeof(jmp_buf));