int
xmlBufMergeBuffer(xmlBufPtr buf, xmlBufferPtr buffer) {
    int ret = 0;

    if ((buf == NULL) || (buf->error)) {
	xmlBufferFree(buffer);
        return(-1);
    }
    CHECK_COMPAT(buf)
    if ((buffer != NULL) && (buffer->content != NULL) &&
             (buffer->length > 0)) {
        ret = xmlBufAdd(buf, buffer->content, buffer->use);
    }
    xmlBufferFree(buffer);
    return(ret);
}

/**
 * xmlBufResetInput:
 * @buf: an xmlBufPtr
 * @input: an xmlParserInputPtr
 *
 * Update the input to use the current set of pointers from the buffer.
 *
 * Returns -1 in case of error, 0 otherwise
 */
int
xmlBufResetInput(xmlBufPtr buf, xmlParserInputPtr input) {
    if (input == NULL)
        return(-1);
    if ((buf == NULL) || (buf->error)) {
        input->base = input->cur = input->end = BAD_CAST "";
        return(-1);
    }
    CHECK_COMPAT(buf)
    input->base = input->cur = buf->content;
    input->end = &buf->content[buf->use];
    return(0);
}

/**
 * xmlBufUpdateInput:
 * @buf: an xmlBufPtr
 * @input: an xmlParserInputPtr
 * @pos: the cur value relative to the beginning of the buffer
 *
 * Update the input to use the base and cur relative to the buffer
 * after a possible reallocation of its content
 *
 * Returns -1 in case of error, 0 otherwise
 */
int
xmlBufUpdateInput(xmlBufPtr buf, xmlParserInputPtr input, size_t pos) {
    if (input == NULL)
        return(-1);
    /*
     * TODO: It might be safer to keep using the buffer content if there
     * was an error.
     */
    if ((buf == NULL) || (buf->error)) {
        input->base = input->cur = input->end = BAD_CAST "";
        return(-1);
    }
    CHECK_COMPAT(buf)
    input->base = buf->content;
    input->cur = input->base + pos;
    input->end = &buf->content[buf->use];
    return(0);
}