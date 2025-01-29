xmlBufPtr buf;
        size_t processed;

        buf = xmlBufCreate();
        if (buf == NULL) {
            xmlErrMemory(ctxt, NULL);
            return(-1);
        }

        /*
         * Shrink the current input buffer.
         * Move it as the raw buffer and create a new input buffer
         */
        processed = inputStream->cur - inputStream->base;
        xmlBufShrink(in->buffer, processed);
        inputStream->consumed += processed;
        in->raw = in->buffer;
        in->buffer = buf;
        in->rawconsumed = processed;