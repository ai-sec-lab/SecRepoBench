int
xmlSwitchInputEncoding(xmlParserCtxtPtr context, xmlParserInputPtr input,
                       xmlCharEncodingHandlerPtr handler)
{
    int nbchars;
    xmlParserInputBufferPtr in;

    if (handler == NULL)
        return (-1);
    if (input == NULL)
        return (-1);
    in = input->buf;
    if (in == NULL) {
	xmlErrInternal(context,
                "static memory buffer doesn't support encoding\n", NULL);
        /*
         * Callers assume that the input buffer takes ownership of the
         * encoding handler. xmlCharEncCloseFunc frees unregistered
         * handlers and avoids a memory leak.
         */
        xmlCharEncCloseFunc(handler);
	return (-1);
    }

    if (in->encoder != NULL) {
        /*
         * TODO: Detect encoding mismatch. We should start by comparing
         * in->encoder->name and handler->name, but there are a few
         * compatible encodings like UTF-16 and UCS-2 or UTF-32 and UCS-4.
         */
        xmlCharEncCloseFunc(handler);
        return (0);
    }

    context->charset = XML_CHAR_ENCODING_UTF8;
    in->encoder = handler;

    /*
     * Is there already some content down the pipe to convert ?
     */
    if (xmlBufIsEmpty(in->buffer) == 0) {
        size_t processed, use, consumed;

        /*
         * Specific handling of the Byte Order Mark for
         * UTF-16
         */
        if ((handler->name != NULL) &&
            (!strcmp(handler->name, "UTF-16LE") ||
             !strcmp(handler->name, "UTF-16")) &&
            (input->cur[0] == 0xFF) && (input->cur[1] == 0xFE)) {
            input->cur += 2;
        }
        if ((handler->name != NULL) &&
            (!strcmp(handler->name, "UTF-16BE")) &&
            (input->cur[0] == 0xFE) && (input->cur[1] == 0xFF)) {
            input->cur += 2;
        }
        /*
         * Errata on XML-1.0 June 20 2001
         * Specific handling of the Byte Order Mark for
         * UTF-8
         */
        if ((handler->name != NULL) &&
            (!strcmp(handler->name, "UTF-8")) &&
            (input->cur[0] == 0xEF) &&
            (input->cur[1] == 0xBB) && (input->cur[2] == 0xBF)) {
            input->cur += 3;
        }

        /*
         * Shrink the current input buffer.
         * Move it as the raw buffer and create a new input buffer
         */
        processed = input->cur - input->base;
        xmlBufShrink(in->buffer, processed);
        input->consumed += processed;
        in->raw = in->buffer;
        in->buffer = xmlBufCreate();
        in->rawconsumed = processed;
        use = xmlBufUse(in->raw);

        nbchars = xmlCharEncInput(in, 0);
        xmlBufResetInput(in->buffer, input);
        // <MASK>
    }
    return (0);
}