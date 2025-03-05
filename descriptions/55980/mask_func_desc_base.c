xmlParseTryOrFinish(xmlParserCtxtPtr ctxt, int terminate) {
    int ret = 0;
    int avail, tlen;
    xmlChar cur, next;

    if (ctxt->input == NULL)
        return(0);

#ifdef DEBUG_PUSH
    switch (ctxt->instate) {
	case XML_PARSER_EOF:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try EOF\n"); break;
	case XML_PARSER_START:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try START\n"); break;
	case XML_PARSER_MISC:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try MISC\n");break;
	case XML_PARSER_COMMENT:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try COMMENT\n");break;
	case XML_PARSER_PROLOG:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try PROLOG\n");break;
	case XML_PARSER_START_TAG:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try START_TAG\n");break;
	case XML_PARSER_CONTENT:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try CONTENT\n");break;
	case XML_PARSER_CDATA_SECTION:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try CDATA_SECTION\n");break;
	case XML_PARSER_END_TAG:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try END_TAG\n");break;
	case XML_PARSER_ENTITY_DECL:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try ENTITY_DECL\n");break;
	case XML_PARSER_ENTITY_VALUE:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try ENTITY_VALUE\n");break;
	case XML_PARSER_ATTRIBUTE_VALUE:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try ATTRIBUTE_VALUE\n");break;
	case XML_PARSER_DTD:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try DTD\n");break;
	case XML_PARSER_EPILOG:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try EPILOG\n");break;
	case XML_PARSER_PI:
	    xmlGenericError(xmlGenericErrorContext,
		    "PP: try PI\n");break;
        case XML_PARSER_IGNORE:
            xmlGenericError(xmlGenericErrorContext,
		    "PP: try IGNORE\n");break;
    }
#endif

    if ((ctxt->input != NULL) &&
        (ctxt->input->cur - ctxt->input->base > 4096)) {
        xmlParserInputShrink(ctxt->input);
    }

    while (ctxt->instate != XML_PARSER_EOF) {
	if ((ctxt->errNo != XML_ERR_OK) && (ctxt->disableSAX == 1))
	    return(0);

	if (ctxt->input == NULL) break;
	if (ctxt->input->buf == NULL)
	    avail = ctxt->input->length -
	            (ctxt->input->cur - ctxt->input->base);
	else {
	    /*
	     * If we are operating on converted input, try to flush
	     * remaining chars to avoid them stalling in the non-converted
	     * buffer. But do not do this in document start where
	     * encoding="..." may not have been read and we work on a
	     * guessed encoding.
	     */
	    if ((ctxt->instate != XML_PARSER_START) &&
	        (ctxt->input->buf->raw != NULL) &&
		(xmlBufIsEmpty(ctxt->input->buf->raw) == 0)) {
                size_t base = xmlBufGetInputBase(ctxt->input->buf->buffer,
                                                 ctxt->input);
		size_t current = ctxt->input->cur - ctxt->input->base;

		xmlParserInputBufferPush(ctxt->input->buf, 0, "");
                xmlBufSetInputBaseCur(ctxt->input->buf->buffer, ctxt->input,
                                      base, current);
	    }
	    avail = xmlBufUse(ctxt->input->buf->buffer) -
		    (ctxt->input->cur - ctxt->input->base);
	}
        if (avail < 1)
	    goto done;
        switch (ctxt->instate) {
            case XML_PARSER_EOF:
	        /*
		 * Document parsing is done !
		 */
	        goto done;
            case XML_PARSER_START:
		if (ctxt->charset == XML_CHAR_ENCODING_NONE) {
		    xmlChar start[4];
		    xmlCharEncoding enc;

		    /*
		     * Very first chars read from the document flow.
		     */
		    if (avail < 4)
			goto done;

		    /*
		     * Get the 4 first bytes and decode the charset
		     * if enc != XML_CHAR_ENCODING_NONE
		     * plug some encoding conversion routines,
		     * else xmlSwitchEncoding will set to (default)
		     * UTF8.
		     */
		    start[0] = RAW;
		    start[1] = NXT(1);
		    start[2] = NXT(2);
		    start[3] = NXT(3);
		    enc = xmlDetectCharEncoding(start, 4);
		    xmlSwitchEncoding(ctxt, enc);
		    break;
		}

		if (avail < 2)
		    goto done;
		cur = ctxt->input->cur[0];
		next = ctxt->input->cur[1];
		if (cur == 0) {
		    if ((ctxt->sax) && (ctxt->sax->setDocumentLocator))
			ctxt->sax->setDocumentLocator(ctxt->userData,
						      &xmlDefaultSAXLocator);
		    xmlFatalErr(ctxt, XML_ERR_DOCUMENT_EMPTY, NULL);
		    xmlHaltParser(ctxt);
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering EOF\n");
#endif
		    if ((ctxt->sax) && (ctxt->sax->endDocument != NULL))
			ctxt->sax->endDocument(ctxt->userData);
		    goto done;
		}
	        if ((cur == '<') && (next == '?')) {
		    /* PI or XML decl */
		    if (avail < 5) goto done;
		    if ((!terminate) &&
                        (!xmlParseLookupString(ctxt, 2, "?>", 2)))
			goto done;
		    if ((ctxt->sax) && (ctxt->sax->setDocumentLocator))
			ctxt->sax->setDocumentLocator(ctxt->userData,
						      &xmlDefaultSAXLocator);
		    if ((ctxt->input->cur[2] == 'x') &&
			(ctxt->input->cur[3] == 'm') &&
			(ctxt->input->cur[4] == 'l') &&
			(IS_BLANK_CH(ctxt->input->cur[5]))) {
			ret += 5;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: Parsing XML Decl\n");
#endif
			xmlParseXMLDecl(ctxt);
			if (ctxt->errNo == XML_ERR_UNSUPPORTED_ENCODING) {
			    /*
			     * The XML REC instructs us to stop parsing right
			     * here
			     */
			    xmlHaltParser(ctxt);
			    return(0);
			}
			ctxt->standalone = ctxt->input->standalone;
			if ((ctxt->encoding == NULL) &&
			    (ctxt->input->encoding != NULL))
			    ctxt->encoding = xmlStrdup(ctxt->input->encoding);
			if ((ctxt->sax) && (ctxt->sax->startDocument) &&
			    (!ctxt->disableSAX))
			    ctxt->sax->startDocument(ctxt->userData);
			ctxt->instate = XML_PARSER_MISC;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering MISC\n");
#endif
		    } else {
			ctxt->version = xmlCharStrdup(XML_DEFAULT_VERSION);
			if ((ctxt->sax) && (ctxt->sax->startDocument) &&
			    (!ctxt->disableSAX))
			    ctxt->sax->startDocument(ctxt->userData);
			ctxt->instate = XML_PARSER_MISC;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering MISC\n");
#endif
		    }
		} else {
		    if ((ctxt->sax) && (ctxt->sax->setDocumentLocator))
			ctxt->sax->setDocumentLocator(ctxt->userData,
						      &xmlDefaultSAXLocator);
		    ctxt->version = xmlCharStrdup(XML_DEFAULT_VERSION);
		    if (ctxt->version == NULL) {
		        xmlErrMemory(ctxt, NULL);
			break;
		    }
		    if ((ctxt->sax) && (ctxt->sax->startDocument) &&
		        (!ctxt->disableSAX))
			ctxt->sax->startDocument(ctxt->userData);
		    ctxt->instate = XML_PARSER_MISC;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering MISC\n");
#endif
		}
		break;
            case XML_PARSER_START_TAG: {
	        const xmlChar *name;
		const xmlChar *prefix = NULL;
		const xmlChar *URI = NULL;
                int line = ctxt->input->line;
		int nsNr = ctxt->nsNr;

		if ((avail < 2) && (ctxt->inputNr == 1))
		    goto done;
		cur = ctxt->input->cur[0];
	        if (cur != '<') {
		    xmlFatalErr(ctxt, XML_ERR_DOCUMENT_EMPTY, NULL);
		    xmlHaltParser(ctxt);
		    if ((ctxt->sax) && (ctxt->sax->endDocument != NULL))
			ctxt->sax->endDocument(ctxt->userData);
		    goto done;
		}
		if ((!terminate) && (!xmlParseLookupGt(ctxt)))
                    goto done;
		if (ctxt->spaceNr == 0)
		    spacePush(ctxt, -1);
		else if (*ctxt->space == -2)
		    spacePush(ctxt, -1);
		else
		    spacePush(ctxt, *ctxt->space);
#ifdef LIBXML_SAX1_ENABLED
		if (ctxt->sax2)
#endif /* LIBXML_SAX1_ENABLED */
		    name = xmlParseStartTag2(ctxt, &prefix, &URI, &tlen);
#ifdef LIBXML_SAX1_ENABLED
		else
		    name = xmlParseStartTag(ctxt);
#endif /* LIBXML_SAX1_ENABLED */
		if (ctxt->instate == XML_PARSER_EOF)
		    goto done;
		if (name == NULL) {
		    spacePop(ctxt);
		    xmlHaltParser(ctxt);
		    if ((ctxt->sax) && (ctxt->sax->endDocument != NULL))
			ctxt->sax->endDocument(ctxt->userData);
		    goto done;
		}
#ifdef LIBXML_VALID_ENABLED
		/*
		 * [ VC: Root Element Type ]
		 * The Name in the document type declaration must match
		 * the element type of the root element.
		 */
		if (ctxt->validate && ctxt->wellFormed && ctxt->myDoc &&
		    ctxt->node && (ctxt->node == ctxt->myDoc->children))
		    ctxt->valid &= xmlValidateRoot(&ctxt->vctxt, ctxt->myDoc);
#endif /* LIBXML_VALID_ENABLED */

		/*
		 * Check for an Empty Element.
		 */
		if ((RAW == '/') && (NXT(1) == '>')) {
		    SKIP(2);

		    if (ctxt->sax2) {
			if ((ctxt->sax != NULL) &&
			    (ctxt->sax->endElementNs != NULL) &&
			    (!ctxt->disableSAX))
			    ctxt->sax->endElementNs(ctxt->userData, name,
			                            prefix, URI);
			if (ctxt->nsNr - nsNr > 0)
			    nsPop(ctxt, ctxt->nsNr - nsNr);
#ifdef LIBXML_SAX1_ENABLED
		    } else {
			if ((ctxt->sax != NULL) &&
			    (ctxt->sax->endElement != NULL) &&
			    (!ctxt->disableSAX))
			    ctxt->sax->endElement(ctxt->userData, name);
#endif /* LIBXML_SAX1_ENABLED */
		    }
		    if (ctxt->instate == XML_PARSER_EOF)
			goto done;
		    spacePop(ctxt);
		    if (ctxt->nameNr == 0) {
			ctxt->instate = XML_PARSER_EPILOG;
		    } else {
			ctxt->instate = XML_PARSER_CONTENT;
		    }
		    break;
		}
		if (RAW == '>') {
		    NEXT;
		} else {
		    xmlFatalErrMsgStr(ctxt, XML_ERR_GT_REQUIRED,
					 "Couldn't find end of Start Tag %s\n",
					 name);
		    nodePop(ctxt);
		    spacePop(ctxt);
		}
                nameNsPush(ctxt, name, prefix, URI, line, ctxt->nsNr - nsNr);

		ctxt->instate = XML_PARSER_CONTENT;
                break;
	    }
            case XML_PARSER_CONTENT: {
		if ((avail < 2) && (ctxt->inputNr == 1))
		    goto done;
		cur = ctxt->input->cur[0];
		next = ctxt->input->cur[1];

		if ((cur == '<') && (next == '/')) {
		    ctxt->instate = XML_PARSER_END_TAG;
		    break;
	        } else if ((cur == '<') && (next == '?')) {
		    if ((!terminate) &&
		        (!xmlParseLookupString(ctxt, 2, "?>", 2)))
			goto done;
		    xmlParsePI(ctxt);
		    ctxt->instate = XML_PARSER_CONTENT;
		} else if ((cur == '<') && (next != '!')) {
		    ctxt->instate = XML_PARSER_START_TAG;
		    break;
		} else if ((cur == '<') && (next == '!') &&
		           (ctxt->input->cur[2] == '-') &&
			   (ctxt->input->cur[3] == '-')) {
		    if ((!terminate) &&
		        (!xmlParseLookupString(ctxt, 4, "-->", 3)))
			goto done;
		    xmlParseComment(ctxt);
		    ctxt->instate = XML_PARSER_CONTENT;
		} else if ((cur == '<') && (ctxt->input->cur[1] == '!') &&
		    (ctxt->input->cur[2] == '[') &&
		    (ctxt->input->cur[3] == 'C') &&
		    (ctxt->input->cur[4] == 'D') &&
		    (ctxt->input->cur[5] == 'A') &&
		    (ctxt->input->cur[6] == 'T') &&
		    (ctxt->input->cur[7] == 'A') &&
		    (ctxt->input->cur[8] == '[')) {
		    SKIP(9);
		    ctxt->instate = XML_PARSER_CDATA_SECTION;
		    break;
		} else if ((cur == '<') && (next == '!') &&
		           (avail < 9)) {
		    goto done;
		} else if (cur == '<') {
		    xmlFatalErr(ctxt, XML_ERR_INTERNAL_ERROR,
		                "detected an error in element content\n");
                    SKIP(1);
		} else if (cur == '&') {
		    if ((!terminate) && (!xmlParseLookupChar(ctxt, ';')))
			goto done;
		    xmlParseReference(ctxt);
		} else {
		    /* TODO Avoid the extra copy, handle directly !!! */
		    /*
		     * Goal of the following test is:
		     *  - minimize calls to the SAX 'character' callback
		     *    when they are mergeable
		     *  - handle an problem for isBlank when we only parse
		     *    a sequence of blank chars and the next one is
		     *    not available to check against '<' presence.
		     *  - tries to homogenize the differences in SAX
		     *    callbacks between the push and pull versions
		     *    of the parser.
		     */
		    if ((ctxt->inputNr == 1) &&
		        (avail < XML_PARSER_BIG_BUFFER_SIZE)) {
			if ((!terminate) && (!xmlParseLookupCharData(ctxt)))
			    goto done;
                    }
                    ctxt->checkIndex = 0;
		    xmlParseCharData(ctxt, 0);
		}
		break;
	    }
            case XML_PARSER_END_TAG:
		if (avail < 2)
		    goto done;
		if ((!terminate) && (!xmlParseLookupChar(ctxt, '>')))
		    goto done;
		if (ctxt->sax2) {
	            xmlParseEndTag2(ctxt, &ctxt->pushTab[ctxt->nameNr - 1]);
		    nameNsPop(ctxt);
		}
#ifdef LIBXML_SAX1_ENABLED
		  else
		    xmlParseEndTag1(ctxt, 0);
#endif /* LIBXML_SAX1_ENABLED */
		if (ctxt->instate == XML_PARSER_EOF) {
		    /* Nothing */
		} else if (ctxt->nameNr == 0) {
		    ctxt->instate = XML_PARSER_EPILOG;
		} else {
		    ctxt->instate = XML_PARSER_CONTENT;
		}
		break;
            case XML_PARSER_CDATA_SECTION: {
	        /*
		 * The Push mode need to have the SAX callback for
		 * cdataBlock merge back contiguous callbacks.
		 */
		const xmlChar *term;

                if (terminate) {
                    /*
                     * Don't call xmlParseLookupString. If 'terminate'
                     * is set, checkIndex is invalid.
                     */
                    term = BAD_CAST strstr((const char *) ctxt->input->cur,
                                           "]]>");
                } else {
		    term = xmlParseLookupString(ctxt, 0, "]]>", 3);
                }

		if (term == NULL) {
		    int tmp, size;

                    if (terminate) {
                        /* Unfinished CDATA section */
                        size = ctxt->input->end - ctxt->input->cur;
                    } else {
                        if (avail < XML_PARSER_BIG_BUFFER_SIZE + 2)
                            goto done;
                        ctxt->checkIndex = 0;
                        /* XXX: Why don't we pass the full buffer? */
                        size = XML_PARSER_BIG_BUFFER_SIZE;
                    }
                    tmp = xmlCheckCdataPush(ctxt->input->cur, size, 0);
                    if (tmp <= 0) {
                        tmp = -tmp;
                        ctxt->input->cur += tmp;
                        goto encoding_error;
                    }
                    if ((ctxt->sax != NULL) && (!ctxt->disableSAX)) {
                        if (ctxt->sax->cdataBlock != NULL)
                            ctxt->sax->cdataBlock(ctxt->userData,
                                                  ctxt->input->cur, tmp);
                        else if (ctxt->sax->characters != NULL)
                            ctxt->sax->characters(ctxt->userData,
                                                  ctxt->input->cur, tmp);
                    }
                    if (ctxt->instate == XML_PARSER_EOF)
                        goto done;
                    SKIPL(tmp);
		} else {
                    int base = term - CUR_PTR;
		    int tmp;

		    tmp = xmlCheckCdataPush(ctxt->input->cur, base, 1);
		    if ((tmp < 0) || (tmp != base)) {
			tmp = -tmp;
			ctxt->input->cur += tmp;
			goto encoding_error;
		    }
		    if ((ctxt->sax != NULL) && (base == 0) &&
		        (ctxt->sax->cdataBlock != NULL) &&
		        (!ctxt->disableSAX)) {
			/*
			 * Special case to provide identical behaviour
			 * between pull and push parsers on enpty CDATA
			 * sections
			 */
			 if ((ctxt->input->cur - ctxt->input->base >= 9) &&
			     (!strncmp((const char *)&ctxt->input->cur[-9],
			               "<![CDATA[", 9)))
			     ctxt->sax->cdataBlock(ctxt->userData,
			                           BAD_CAST "", 0);
		    } else if ((ctxt->sax != NULL) && (base > 0) &&
			(!ctxt->disableSAX)) {
			if (ctxt->sax->cdataBlock != NULL)
			    ctxt->sax->cdataBlock(ctxt->userData,
						  ctxt->input->cur, base);
			else if (ctxt->sax->characters != NULL)
			    ctxt->sax->characters(ctxt->userData,
						  ctxt->input->cur, base);
		    }
		    if (ctxt->instate == XML_PARSER_EOF)
			goto done;
		    SKIPL(base + 3);
		    ctxt->instate = XML_PARSER_CONTENT;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering CONTENT\n");
#endif
		}
		break;
	    }
            case XML_PARSER_MISC:
            case XML_PARSER_PROLOG:
            case XML_PARSER_EPILOG:
		SKIP_BLANKS;
		if (ctxt->input->buf == NULL)
		    avail = ctxt->input->length -
		            (ctxt->input->cur - ctxt->input->base);
		else
		    avail = xmlBufUse(ctxt->input->buf->buffer) -
		            (ctxt->input->cur - ctxt->input->base);
		if (avail < 2)
		    goto done;
		cur = ctxt->input->cur[0];
		next = ctxt->input->cur[1];
	        if ((cur == '<') && (next == '?')) {
		    if ((!terminate) &&
                        (!xmlParseLookupString(ctxt, 2, "?>", 2)))
			goto done;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: Parsing PI\n");
#endif
		    xmlParsePI(ctxt);
		    if (ctxt->instate == XML_PARSER_EOF)
			goto done;
		} else if ((cur == '<') && (next == '!') &&
		    (ctxt->input->cur[2] == '-') &&
		    (ctxt->input->cur[3] == '-')) {
		    if ((!terminate) &&
                        (!xmlParseLookupString(ctxt, 4, "-->", 3)))
			goto done;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: Parsing Comment\n");
#endif
		    xmlParseComment(ctxt);
		    if (ctxt->instate == XML_PARSER_EOF)
			goto done;
		} else if ((ctxt->instate == XML_PARSER_MISC) &&
                    (cur == '<') && (next == '!') &&
		    (ctxt->input->cur[2] == 'D') &&
		    (ctxt->input->cur[3] == 'O') &&
		    (ctxt->input->cur[4] == 'C') &&
		    (ctxt->input->cur[5] == 'T') &&
		    (ctxt->input->cur[6] == 'Y') &&
		    (ctxt->input->cur[7] == 'P') &&
		    (ctxt->input->cur[8] == 'E')) {
		    if ((!terminate) && (!xmlParseLookupGt(ctxt)))
                        goto done;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: Parsing internal subset\n");
#endif
		    ctxt->inSubset = 1;
		    xmlParseDocTypeDecl(ctxt);
		    if (ctxt->instate == XML_PARSER_EOF)
			goto done;
		    if (RAW == '[') {
			ctxt->instate = XML_PARSER_DTD;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering DTD\n");
#endif
		    } else {
			/*
			 * Create and update the external subset.
			 */
			ctxt->inSubset = 2;
			if ((ctxt->sax != NULL) && (!ctxt->disableSAX) &&
			    (ctxt->sax->externalSubset != NULL))
			    ctxt->sax->externalSubset(ctxt->userData,
				    ctxt->intSubName, ctxt->extSubSystem,
				    ctxt->extSubURI);
			ctxt->inSubset = 0;
			xmlCleanSpecialAttr(ctxt);
			ctxt->instate = XML_PARSER_PROLOG;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering PROLOG\n");
#endif
		    }
		} else if ((cur == '<') && (next == '!') &&
		           (avail <
                            (ctxt->instate == XML_PARSER_MISC ? 9 : 4))) {
		    goto done;
		} else if (ctxt->instate == XML_PARSER_EPILOG) {
		    xmlFatalErr(ctxt, XML_ERR_DOCUMENT_END, NULL);
		    xmlHaltParser(ctxt);
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering EOF\n");
#endif
		    if ((ctxt->sax) && (ctxt->sax->endDocument != NULL))
			ctxt->sax->endDocument(ctxt->userData);
		    goto done;
                } else {
		    ctxt->instate = XML_PARSER_START_TAG;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering START_TAG\n");
#endif
		}
		break;
            case XML_PARSER_DTD: {
                if ((!terminate) && (!xmlParseLookupInternalSubset(ctxt)))
                    goto done;
		xmlParseInternalSubset(ctxt);
		if (ctxt->instate == XML_PARSER_EOF)
		    goto done;
		ctxt->inSubset = 2;
		if ((ctxt->sax != NULL) && (!ctxt->disableSAX) &&
		    (ctxt->sax->externalSubset != NULL))
		    ctxt->sax->externalSubset(ctxt->userData, ctxt->intSubName,
			    ctxt->extSubSystem, ctxt->extSubURI);
		ctxt->inSubset = 0;
		xmlCleanSpecialAttr(ctxt);
		if (ctxt->instate == XML_PARSER_EOF)
		    goto done;
		ctxt->instate = XML_PARSER_PROLOG;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering PROLOG\n");
#endif
                break;
	    }
            case XML_PARSER_COMMENT:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == COMMENT\n");
		ctxt->instate = XML_PARSER_CONTENT;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering CONTENT\n");
#endif
		break;
            case XML_PARSER_IGNORE:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == IGNORE");
	        ctxt->instate = XML_PARSER_DTD;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering DTD\n");
#endif
	        break;
            case XML_PARSER_PI:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == PI\n");
		ctxt->instate = XML_PARSER_CONTENT;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering CONTENT\n");
#endif
		break;
            case XML_PARSER_ENTITY_DECL:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == ENTITY_DECL\n");
		ctxt->instate = XML_PARSER_DTD;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering DTD\n");
#endif
		break;
            case XML_PARSER_ENTITY_VALUE:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == ENTITY_VALUE\n");
		ctxt->instate = XML_PARSER_CONTENT;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering DTD\n");
#endif
		break;
            case XML_PARSER_ATTRIBUTE_VALUE:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == ATTRIBUTE_VALUE\n");
		ctxt->instate = XML_PARSER_START_TAG;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering START_TAG\n");
#endif
		break;
            case XML_PARSER_SYSTEM_LITERAL:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == SYSTEM_LITERAL\n");
		ctxt->instate = XML_PARSER_START_TAG;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering START_TAG\n");
#endif
		break;
            case XML_PARSER_PUBLIC_LITERAL:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == PUBLIC_LITERAL\n");
		ctxt->instate = XML_PARSER_START_TAG;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering START_TAG\n");
#endif
		break;
	}
    }
done:
#ifdef DEBUG_PUSH
    xmlGenericError(xmlGenericErrorContext, "PP: done %d\n", ret);
#endif
    return(ret);
// Handle encoding errors encountered during XML parsing.
// Check if the input does not conform to UTF-8 encoding.
// Report an encoding error with details about the problematic bytes.
// Terminate the parsing process upon encountering this error.
// <MASK>
}