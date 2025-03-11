if (nbchar >= HTML_PARSER_BIG_BUFFER_SIZE) {
            buf[nbchar] = 0;
	    if (ctxt->sax->cdataBlock!= NULL) {
		/*
		 * Insert as CDATA, which is the same as HTML_PRESERVE_NODE
		 */
		ctxt->sax->cdataBlock(ctxt->userData, buf, nbchar);
	    } else if (ctxt->sax->characters != NULL) {
		ctxt->sax->characters(ctxt->userData, buf, nbchar);
	    }
	    nbchar = 0;
	}
	NEXTL(l);
	GROW;
	currentChar = CUR_CHAR(l);