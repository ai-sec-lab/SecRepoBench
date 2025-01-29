oSrc.term_source = RMFJPEGNoop;
    oSrc.init_source = RMFJPEGNoop;
    oSrc.fill_input_buffer = RMFJPEG_fill_input_buffer_dec;
    oSrc.skip_input_data = RMFJPEG_skip_input_data_dec;

    jpeg_create_decompress(&oJpegInfo);

    memset(&oJmpBuf, 0, sizeof(jmp_buf));