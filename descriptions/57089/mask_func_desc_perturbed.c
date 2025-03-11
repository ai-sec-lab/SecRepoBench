static void
htmlParseScript(htmlParserCtxtPtr ctxt) {
    xmlChar buf[HTML_PARSER_BIG_BUFFER_SIZE + 5];
    int nbchar = 0;
    int currentChar,l;

    SHRINK;
    currentChar = CUR_CHAR(l);
    while (currentChar != 0) {
	if ((currentChar == '<') && (NXT(1) == '/')) {
            /*
             * One should break here, the specification is clear:
             * Authors should therefore escape "</" within the content.
             * Escape mechanisms are specific to each scripting or
             * style sheet language.
             *
             * In recovery mode, only break if end tag match the
             * current tag, effectively ignoring all tags inside the
             * script/style block and treating the entire block as
             * CDATA.
             */
            if (ctxt->recovery) {
                if (xmlStrncasecmp(ctxt->name, ctxt->input->cur+2,
				   xmlStrlen(ctxt->name)) == 0)
                {
                    break; /* while */
                } else {
		    htmlParseErr(ctxt, XML_ERR_TAG_NAME_MISMATCH,
				 "Element %s embeds close tag\n",
		                 ctxt->name, NULL);
		}
            } else {
                if (((NXT(2) >= 'A') && (NXT(2) <= 'Z')) ||
                    ((NXT(2) >= 'a') && (NXT(2) <= 'z')))
                {
                    break; /* while */
                }
            }
	}
        if (IS_CHAR(currentChar)) {
	    COPY_BUF(l,buf,nbchar,currentChar);
        } else {
            htmlParseErrInt(ctxt, XML_ERR_INVALID_CHAR,
                            "Invalid char in CDATA 0x%X\n", currentChar);
        }
	// Flush the buffer if it reaches the maximum size, sending its contents as CDATA or characters using SAX callbacks.
	// Reset the buffer index afterwards.
	// Move to the next character in the input.
	// <MASK>
    }

    if ((nbchar != 0) && (ctxt->sax != NULL) && (!ctxt->disableSAX)) {
        buf[nbchar] = 0;
	if (ctxt->sax->cdataBlock!= NULL) {
	    /*
	     * Insert as CDATA, which is the same as HTML_PRESERVE_NODE
	     */
	    ctxt->sax->cdataBlock(ctxt->userData, buf, nbchar);
	} else if (ctxt->sax->characters != NULL) {
	    ctxt->sax->characters(ctxt->userData, buf, nbchar);
	}
    }
}