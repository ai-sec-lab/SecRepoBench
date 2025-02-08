static xmlCharEncodingHandlerPtr
xmlDetectEBCDIC(xmlParserInputPtr input) {
    xmlChar out[200];
    xmlCharEncodingHandlerPtr handler;
    int inlen, outlen, result, i;

    /*
     * To detect the EBCDIC code page, we convert the first 200 bytes
     * to EBCDIC-US and try to find the encoding declaration.
     */
    handler = xmlGetCharEncodingHandler(XML_CHAR_ENCODING_EBCDIC);
    if (handler == NULL)
        return(NULL);
    // <MASK>

    for (i = 0; i < outlen; i++) {
        if (out[i] == '>')
            break;
        if ((out[i] == 'e') &&
            (xmlStrncmp(out + i, BAD_CAST "encoding", 8) == 0)) {
            int start, cur, quote;

            i += 8;
            while (IS_BLANK_CH(out[i]))
                i += 1;
            if (out[i++] != '=')
                break;
            while (IS_BLANK_CH(out[i]))
                i += 1;
            quote = out[i++];
            if ((quote != '\'') && (quote != '"'))
                break;
            start = i;
            cur = out[i];
            while (((cur >= 'a') && (cur <= 'z')) ||
                   ((cur >= 'A') && (cur <= 'Z')) ||
                   ((cur >= '0') && (cur <= '9')) ||
                   (cur == '.') || (cur == '_') ||
                   (cur == '-'))
                cur = out[++i];
            if (cur != quote)
                break;
            out[i] = 0;
            xmlCharEncCloseFunc(handler);
            handler = xmlFindCharEncodingHandler((char *) out + start);
            break;
        }
    }

    return(handler);
}