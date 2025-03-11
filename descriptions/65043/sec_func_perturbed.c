int
xmlSwitchInputEncoding(xmlParserCtxtPtr ctxt, xmlParserInputPtr inputStream,
                       xmlCharEncodingHandlerPtr handler)
{
    int nbchars;
    xmlParserInputBufferPtr in;

    if ((inputStream == NULL) || (inputStream->buf == NULL)) {
        xmlCharEncCloseFunc(handler);
	return (-1);
    }
    in = inputStream->buf;

    inputStream->flags |= XML_INPUT_HAS_ENCODING;

    /*
     * UTF-8 requires no encoding handler.
     */
    if ((handler != NULL) &&
        (xmlStrcasecmp(BAD_CAST handler->name, BAD_CAST "UTF-8") == 0)) {
        xmlCharEncCloseFunc(handler);
        handler = NULL;
    }

    if (in->encoder == handler)
        return (0);

    if (in->encoder != NULL) {
        /*
         * Switching encodings during parsing is a really bad idea,
         * but Chromium can switch between ISO-8859-1 and UTF-16 before
         * separate calls to xmlParseChunk.
         *
         * TODO: We should check whether the "raw" input buffer is empty and
         * convert the old content using the old encoder.
         */

        xmlCharEncCloseFunc(in->encoder);
        in->encoder = handler;
        return (0);
    }

    in->encoder = handler;

    /*
     * Is there already some content down the pipe to convert ?
     */
    if (xmlBufIsEmpty(in->buffer) == 0) {
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

        nbchars = xmlCharEncInput(in);
        xmlBufResetInput(in->buffer, inputStream);
        if (nbchars == XML_ENC_ERR_MEMORY) {
            xmlErrMemory(ctxt, NULL);
        } else if (nbchars < 0) {
            xmlErrInternal(ctxt,
                           "switching encoding: encoder error\n",
                           NULL);
            xmlHaltParser(ctxt);
            return (-1);
        }
    }
    return (0);
}