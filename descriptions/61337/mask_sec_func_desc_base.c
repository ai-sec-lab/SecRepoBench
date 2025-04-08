static int
htmlCurrentChar(xmlParserCtxtPtr ctxt, int *len) {
    const unsigned char *cur;
    unsigned char c;
    unsigned int val;

    if (ctxt->instate == XML_PARSER_EOF)
	return(0);

    if (ctxt->token != 0) {
	*len = 0;
	return(ctxt->token);
    }

    if (ctxt->input->end - ctxt->input->cur < INPUT_CHUNK) {
        xmlParserGrow(ctxt);
        if (ctxt->instate == XML_PARSER_EOF)
            return(0);
    }

    if ((ctxt->input->flags & XML_INPUT_HAS_ENCODING) == 0) {
        xmlChar * guess;
        xmlCharEncodingHandlerPtr handler;

        /*
         * Assume it's a fixed length encoding (1) with
         * a compatible encoding for the ASCII set, since
         * HTML constructs only use < 128 chars
         */
        if (*ctxt->input->cur < 0x80) {
            *len = 1;
            if ((*ctxt->input->cur == 0) &&
                (ctxt->input->cur < ctxt->input->end)) {
                htmlParseErrInt(ctxt, XML_ERR_INVALID_CHAR,
                                "Char 0x%X out of allowed range\n", 0);
                return(' ');
            }
            return(*ctxt->input->cur);
        }

        /*
         * Humm this is bad, do an automatic flow conversion
         */
        guess = htmlFindEncoding(ctxt);
        if (guess == NULL) {
            xmlSwitchEncoding(ctxt, XML_CHAR_ENCODING_8859_1);
        } else 
        // Find a character encoding handler based on a guessed encoding.
        // If the handler is found and is not "UTF-8", switch the parser's encoding
        // to use this handler. If no handler is found, report an unsupported
        // encoding error.
        // <MASK>
        ctxt->input->flags |= XML_INPUT_HAS_ENCODING;
    }

    /*
     * We are supposed to handle UTF8, check it's valid
     * From rfc2044: encoding of the Unicode values on UTF-8:
     *
     * UCS-4 range (hex.)           UTF-8 octet sequence (binary)
     * 0000 0000-0000 007F   0xxxxxxx
     * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
     * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
     *
     * Check for the 0x110000 limit too
     */
    cur = ctxt->input->cur;
    c = *cur;
    if (c & 0x80) {
        size_t avail;

        if ((c & 0x40) == 0)
            goto encoding_error;

        avail = ctxt->input->end - ctxt->input->cur;

        if ((avail < 2) || ((cur[1] & 0xc0) != 0x80))
            goto encoding_error;
        if ((c & 0xe0) == 0xe0) {
            if ((avail < 3) || ((cur[2] & 0xc0) != 0x80))
                goto encoding_error;
            if ((c & 0xf0) == 0xf0) {
                if (((c & 0xf8) != 0xf0) ||
                    (avail < 4) || ((cur[3] & 0xc0) != 0x80))
                    goto encoding_error;
                /* 4-byte code */
                *len = 4;
                val = (cur[0] & 0x7) << 18;
                val |= (cur[1] & 0x3f) << 12;
                val |= (cur[2] & 0x3f) << 6;
                val |= cur[3] & 0x3f;
                if (val < 0x10000)
                    goto encoding_error;
            } else {
              /* 3-byte code */
                *len = 3;
                val = (cur[0] & 0xf) << 12;
                val |= (cur[1] & 0x3f) << 6;
                val |= cur[2] & 0x3f;
                if (val < 0x800)
                    goto encoding_error;
            }
        } else {
          /* 2-byte code */
            *len = 2;
            val = (cur[0] & 0x1f) << 6;
            val |= cur[1] & 0x3f;
            if (val < 0x80)
                goto encoding_error;
        }
        if (!IS_CHAR(val)) {
            htmlParseErrInt(ctxt, XML_ERR_INVALID_CHAR,
                            "Char 0x%X out of allowed range\n", val);
        }
        return(val);
    } else {
        if ((*ctxt->input->cur == 0) &&
            (ctxt->input->cur < ctxt->input->end)) {
            htmlParseErrInt(ctxt, XML_ERR_INVALID_CHAR,
                            "Char 0x%X out of allowed range\n", 0);
            *len = 1;
            return(' ');
        }
        /* 1-byte code */
        *len = 1;
        return(*ctxt->input->cur);
    }

encoding_error:
    {
        char buffer[150];

	if (ctxt->input->end - ctxt->input->cur >= 4) {
	    snprintf(buffer, 149, "Bytes: 0x%02X 0x%02X 0x%02X 0x%02X\n",
			    ctxt->input->cur[0], ctxt->input->cur[1],
			    ctxt->input->cur[2], ctxt->input->cur[3]);
	} else {
	    snprintf(buffer, 149, "Bytes: 0x%02X\n", ctxt->input->cur[0]);
	}
	htmlParseErr(ctxt, XML_ERR_INVALID_ENCODING,
		     "Input is not proper UTF-8, indicate encoding !\n",
		     BAD_CAST buffer, NULL);
    }

    if ((ctxt->input->flags & XML_INPUT_HAS_ENCODING) == 0)
        xmlSwitchEncoding(ctxt, XML_CHAR_ENCODING_8859_1);
    *len = 1;
    return(*ctxt->input->cur);
}