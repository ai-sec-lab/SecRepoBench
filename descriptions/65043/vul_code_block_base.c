size_t processed;

        /*
         * Shrink the current input buffer.
         * Move it as the raw buffer and create a new input buffer
         */
        processed = input->cur - input->base;
        xmlBufShrink(in->buffer, processed);
        input->consumed += processed;
        in->raw = in->buffer;
        in->buffer = xmlBufCreate();
        if (in->buffer == NULL) {
            xmlErrMemory(ctxt, NULL);
            return(-1);
        }
        in->rawconsumed = processed;