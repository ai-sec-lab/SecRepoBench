xmlParseTryOrFinish(xmlParserCtxtPtr parserContext, int terminate) {
    int ret = 0;
    int avail, tlen;
    xmlChar cur, next;

    if (parserContext->input == NULL)
        return(0);

#ifdef DEBUG_PUSH
    switch (parserContext->instate) {
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

    if ((parserContext->input != NULL) &&
        (parserContext->input->cur - parserContext->input->base > 4096)) {
        xmlParserInputShrink(parserContext->input);
    }

    while (parserContext->instate != XML_PARSER_EOF) {
	if ((parserContext->errNo != XML_ERR_OK) && (parserContext->disableSAX == 1))
	    return(0);

	if (parserContext->input == NULL) break;
	if (parserContext->input->buf == NULL)
	    avail = parserContext->input->length -
	            (parserContext->input->cur - parserContext->input->base);
	else {
	    /*
	     * If we are operating on converted input, try to flush
	     * remaining chars to avoid them stalling in the non-converted
	     * buffer. But do not do this in document start where
	     * encoding="..." may not have been read and we work on a
	     * guessed encoding.
	     */
	    if ((parserContext->instate != XML_PARSER_START) &&
	        (parserContext->input->buf->raw != NULL) &&
		(xmlBufIsEmpty(parserContext->input->buf->raw) == 0)) {
                size_t base = xmlBufGetInputBase(parserContext->input->buf->buffer,
                                                 parserContext->input);
		size_t current = parserContext->input->cur - parserContext->input->base;

		xmlParserInputBufferPush(parserContext->input->buf, 0, "");
                xmlBufSetInputBaseCur(parserContext->input->buf->buffer, parserContext->input,
                                      base, current);
	    }
	    avail = xmlBufUse(parserContext->input->buf->buffer) -
		    (parserContext->input->cur - parserContext->input->base);
	}
        if (avail < 1)
	    goto done;
        switch (parserContext->instate) {
            case XML_PARSER_EOF:
	        /*
		 * Document parsing is done !
		 */
	        goto done;
            case XML_PARSER_START:
		if (parserContext->charset == XML_CHAR_ENCODING_NONE) {
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
		    xmlSwitchEncoding(parserContext, enc);
		    break;
		}

		if (avail < 2)
		    goto done;
		cur = parserContext->input->cur[0];
		next = parserContext->input->cur[1];
		if (cur == 0) {
		    if ((parserContext->sax) && (parserContext->sax->setDocumentLocator))
			parserContext->sax->setDocumentLocator(parserContext->userData,
						      &xmlDefaultSAXLocator);
		    xmlFatalErr(parserContext, XML_ERR_DOCUMENT_EMPTY, NULL);
		    xmlHaltParser(parserContext);
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering EOF\n");
#endif
		    if ((parserContext->sax) && (parserContext->sax->endDocument != NULL))
			parserContext->sax->endDocument(parserContext->userData);
		    goto done;
		}
	        if ((cur == '<') && (next == '?')) {
		    /* PI or XML decl */
		    if (avail < 5) goto done;
		    if ((!terminate) &&
                        (!xmlParseLookupString(parserContext, 2, "?>", 2)))
			goto done;
		    if ((parserContext->sax) && (parserContext->sax->setDocumentLocator))
			parserContext->sax->setDocumentLocator(parserContext->userData,
						      &xmlDefaultSAXLocator);
		    if ((parserContext->input->cur[2] == 'x') &&
			(parserContext->input->cur[3] == 'm') &&
			(parserContext->input->cur[4] == 'l') &&
			(IS_BLANK_CH(parserContext->input->cur[5]))) {
			ret += 5;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: Parsing XML Decl\n");
#endif
			xmlParseXMLDecl(parserContext);
			if (parserContext->errNo == XML_ERR_UNSUPPORTED_ENCODING) {
			    /*
			     * The XML REC instructs us to stop parsing right
			     * here
			     */
			    xmlHaltParser(parserContext);
			    return(0);
			}
			parserContext->standalone = parserContext->input->standalone;
			if ((parserContext->encoding == NULL) &&
			    (parserContext->input->encoding != NULL))
			    parserContext->encoding = xmlStrdup(parserContext->input->encoding);
			if ((parserContext->sax) && (parserContext->sax->startDocument) &&
			    (!parserContext->disableSAX))
			    parserContext->sax->startDocument(parserContext->userData);
			parserContext->instate = XML_PARSER_MISC;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering MISC\n");
#endif
		    } else {
			parserContext->version = xmlCharStrdup(XML_DEFAULT_VERSION);
			if ((parserContext->sax) && (parserContext->sax->startDocument) &&
			    (!parserContext->disableSAX))
			    parserContext->sax->startDocument(parserContext->userData);
			parserContext->instate = XML_PARSER_MISC;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering MISC\n");
#endif
		    }
		} else {
		    if ((parserContext->sax) && (parserContext->sax->setDocumentLocator))
			parserContext->sax->setDocumentLocator(parserContext->userData,
						      &xmlDefaultSAXLocator);
		    parserContext->version = xmlCharStrdup(XML_DEFAULT_VERSION);
		    if (parserContext->version == NULL) {
		        xmlErrMemory(parserContext, NULL);
			break;
		    }
		    if ((parserContext->sax) && (parserContext->sax->startDocument) &&
		        (!parserContext->disableSAX))
			parserContext->sax->startDocument(parserContext->userData);
		    parserContext->instate = XML_PARSER_MISC;
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
                int line = parserContext->input->line;
		int nsNr = parserContext->nsNr;

		if ((avail < 2) && (parserContext->inputNr == 1))
		    goto done;
		cur = parserContext->input->cur[0];
	        if (cur != '<') {
		    xmlFatalErr(parserContext, XML_ERR_DOCUMENT_EMPTY, NULL);
		    xmlHaltParser(parserContext);
		    if ((parserContext->sax) && (parserContext->sax->endDocument != NULL))
			parserContext->sax->endDocument(parserContext->userData);
		    goto done;
		}
		if ((!terminate) && (!xmlParseLookupGt(parserContext)))
                    goto done;
		if (parserContext->spaceNr == 0)
		    spacePush(parserContext, -1);
		else if (*parserContext->space == -2)
		    spacePush(parserContext, -1);
		else
		    spacePush(parserContext, *parserContext->space);
#ifdef LIBXML_SAX1_ENABLED
		if (parserContext->sax2)
#endif /* LIBXML_SAX1_ENABLED */
		    name = xmlParseStartTag2(parserContext, &prefix, &URI, &tlen);
#ifdef LIBXML_SAX1_ENABLED
		else
		    name = xmlParseStartTag(parserContext);
#endif /* LIBXML_SAX1_ENABLED */
		if (parserContext->instate == XML_PARSER_EOF)
		    goto done;
		if (name == NULL) {
		    spacePop(parserContext);
		    xmlHaltParser(parserContext);
		    if ((parserContext->sax) && (parserContext->sax->endDocument != NULL))
			parserContext->sax->endDocument(parserContext->userData);
		    goto done;
		}
#ifdef LIBXML_VALID_ENABLED
		/*
		 * [ VC: Root Element Type ]
		 * The Name in the document type declaration must match
		 * the element type of the root element.
		 */
		if (parserContext->validate && parserContext->wellFormed && parserContext->myDoc &&
		    parserContext->node && (parserContext->node == parserContext->myDoc->children))
		    parserContext->valid &= xmlValidateRoot(&parserContext->vctxt, parserContext->myDoc);
#endif /* LIBXML_VALID_ENABLED */

		/*
		 * Check for an Empty Element.
		 */
		if ((RAW == '/') && (NXT(1) == '>')) {
		    SKIP(2);

		    if (parserContext->sax2) {
			if ((parserContext->sax != NULL) &&
			    (parserContext->sax->endElementNs != NULL) &&
			    (!parserContext->disableSAX))
			    parserContext->sax->endElementNs(parserContext->userData, name,
			                            prefix, URI);
			if (parserContext->nsNr - nsNr > 0)
			    nsPop(parserContext, parserContext->nsNr - nsNr);
#ifdef LIBXML_SAX1_ENABLED
		    } else {
			if ((parserContext->sax != NULL) &&
			    (parserContext->sax->endElement != NULL) &&
			    (!parserContext->disableSAX))
			    parserContext->sax->endElement(parserContext->userData, name);
#endif /* LIBXML_SAX1_ENABLED */
		    }
		    if (parserContext->instate == XML_PARSER_EOF)
			goto done;
		    spacePop(parserContext);
		    if (parserContext->nameNr == 0) {
			parserContext->instate = XML_PARSER_EPILOG;
		    } else {
			parserContext->instate = XML_PARSER_CONTENT;
		    }
		    break;
		}
		if (RAW == '>') {
		    NEXT;
		} else {
		    xmlFatalErrMsgStr(parserContext, XML_ERR_GT_REQUIRED,
					 "Couldn't find end of Start Tag %s\n",
					 name);
		    nodePop(parserContext);
		    spacePop(parserContext);
		}
                nameNsPush(parserContext, name, prefix, URI, line, parserContext->nsNr - nsNr);

		parserContext->instate = XML_PARSER_CONTENT;
                break;
	    }
            case XML_PARSER_CONTENT: {
		if ((avail < 2) && (parserContext->inputNr == 1))
		    goto done;
		cur = parserContext->input->cur[0];
		next = parserContext->input->cur[1];

		if ((cur == '<') && (next == '/')) {
		    parserContext->instate = XML_PARSER_END_TAG;
		    break;
	        } else if ((cur == '<') && (next == '?')) {
		    if ((!terminate) &&
		        (!xmlParseLookupString(parserContext, 2, "?>", 2)))
			goto done;
		    xmlParsePI(parserContext);
		    parserContext->instate = XML_PARSER_CONTENT;
		} else if ((cur == '<') && (next != '!')) {
		    parserContext->instate = XML_PARSER_START_TAG;
		    break;
		} else if ((cur == '<') && (next == '!') &&
		           (parserContext->input->cur[2] == '-') &&
			   (parserContext->input->cur[3] == '-')) {
		    if ((!terminate) &&
		        (!xmlParseLookupString(parserContext, 4, "-->", 3)))
			goto done;
		    xmlParseComment(parserContext);
		    parserContext->instate = XML_PARSER_CONTENT;
		} else if ((cur == '<') && (parserContext->input->cur[1] == '!') &&
		    (parserContext->input->cur[2] == '[') &&
		    (parserContext->input->cur[3] == 'C') &&
		    (parserContext->input->cur[4] == 'D') &&
		    (parserContext->input->cur[5] == 'A') &&
		    (parserContext->input->cur[6] == 'T') &&
		    (parserContext->input->cur[7] == 'A') &&
		    (parserContext->input->cur[8] == '[')) {
		    SKIP(9);
		    parserContext->instate = XML_PARSER_CDATA_SECTION;
		    break;
		} else if ((cur == '<') && (next == '!') &&
		           (avail < 9)) {
		    goto done;
		} else if (cur == '<') {
		    xmlFatalErr(parserContext, XML_ERR_INTERNAL_ERROR,
		                "detected an error in element content\n");
                    SKIP(1);
		} else if (cur == '&') {
		    if ((!terminate) && (!xmlParseLookupChar(parserContext, ';')))
			goto done;
		    xmlParseReference(parserContext);
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
		    if ((parserContext->inputNr == 1) &&
		        (avail < XML_PARSER_BIG_BUFFER_SIZE)) {
			if ((!terminate) && (!xmlParseLookupCharData(parserContext)))
			    goto done;
                    }
                    parserContext->checkIndex = 0;
		    xmlParseCharData(parserContext, 0);
		}
		break;
	    }
            case XML_PARSER_END_TAG:
		if (avail < 2)
		    goto done;
		if ((!terminate) && (!xmlParseLookupChar(parserContext, '>')))
		    goto done;
		if (parserContext->sax2) {
	            xmlParseEndTag2(parserContext, &parserContext->pushTab[parserContext->nameNr - 1]);
		    nameNsPop(parserContext);
		}
#ifdef LIBXML_SAX1_ENABLED
		  else
		    xmlParseEndTag1(ctxt, 0);
#endif /* LIBXML_SAX1_ENABLED */
		if (parserContext->instate == XML_PARSER_EOF) {
		    /* Nothing */
		} else if (parserContext->nameNr == 0) {
		    parserContext->instate = XML_PARSER_EPILOG;
		} else {
		    parserContext->instate = XML_PARSER_CONTENT;
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
                    term = BAD_CAST strstr((const char *) parserContext->input->cur,
                                           "]]>");
                } else {
		    term = xmlParseLookupString(parserContext, 0, "]]>", 3);
                }

		if (term == NULL) {
		    int tmp, size;

                    if (terminate) {
                        /* Unfinished CDATA section */
                        size = parserContext->input->end - parserContext->input->cur;
                    } else {
                        if (avail < XML_PARSER_BIG_BUFFER_SIZE + 2)
                            goto done;
                        parserContext->checkIndex = 0;
                        /* XXX: Why don't we pass the full buffer? */
                        size = XML_PARSER_BIG_BUFFER_SIZE;
                    }
                    tmp = xmlCheckCdataPush(parserContext->input->cur, size, 0);
                    if (tmp <= 0) {
                        tmp = -tmp;
                        parserContext->input->cur += tmp;
                        goto encoding_error;
                    }
                    if ((parserContext->sax != NULL) && (!parserContext->disableSAX)) {
                        if (parserContext->sax->cdataBlock != NULL)
                            parserContext->sax->cdataBlock(parserContext->userData,
                                                  parserContext->input->cur, tmp);
                        else if (parserContext->sax->characters != NULL)
                            parserContext->sax->characters(parserContext->userData,
                                                  parserContext->input->cur, tmp);
                    }
                    if (parserContext->instate == XML_PARSER_EOF)
                        goto done;
                    SKIPL(tmp);
		} else {
                    int base = term - CUR_PTR;
		    int tmp;

		    tmp = xmlCheckCdataPush(parserContext->input->cur, base, 1);
		    if ((tmp < 0) || (tmp != base)) {
			tmp = -tmp;
			parserContext->input->cur += tmp;
			goto encoding_error;
		    }
		    if ((parserContext->sax != NULL) && (base == 0) &&
		        (parserContext->sax->cdataBlock != NULL) &&
		        (!parserContext->disableSAX)) {
			/*
			 * Special case to provide identical behaviour
			 * between pull and push parsers on enpty CDATA
			 * sections
			 */
			 if ((parserContext->input->cur - parserContext->input->base >= 9) &&
			     (!strncmp((const char *)&parserContext->input->cur[-9],
			               "<![CDATA[", 9)))
			     parserContext->sax->cdataBlock(parserContext->userData,
			                           BAD_CAST "", 0);
		    } else if ((parserContext->sax != NULL) && (base > 0) &&
			(!parserContext->disableSAX)) {
			if (parserContext->sax->cdataBlock != NULL)
			    parserContext->sax->cdataBlock(parserContext->userData,
						  parserContext->input->cur, base);
			else if (parserContext->sax->characters != NULL)
			    parserContext->sax->characters(parserContext->userData,
						  parserContext->input->cur, base);
		    }
		    if (parserContext->instate == XML_PARSER_EOF)
			goto done;
		    SKIPL(base + 3);
		    parserContext->instate = XML_PARSER_CONTENT;
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
		if (parserContext->input->buf == NULL)
		    avail = parserContext->input->length -
		            (parserContext->input->cur - parserContext->input->base);
		else
		    avail = xmlBufUse(parserContext->input->buf->buffer) -
		            (parserContext->input->cur - parserContext->input->base);
		if (avail < 2)
		    goto done;
		cur = parserContext->input->cur[0];
		next = parserContext->input->cur[1];
	        if ((cur == '<') && (next == '?')) {
		    if ((!terminate) &&
                        (!xmlParseLookupString(parserContext, 2, "?>", 2)))
			goto done;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: Parsing PI\n");
#endif
		    xmlParsePI(parserContext);
		    if (parserContext->instate == XML_PARSER_EOF)
			goto done;
		} else if ((cur == '<') && (next == '!') &&
		    (parserContext->input->cur[2] == '-') &&
		    (parserContext->input->cur[3] == '-')) {
		    if ((!terminate) &&
                        (!xmlParseLookupString(parserContext, 4, "-->", 3)))
			goto done;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: Parsing Comment\n");
#endif
		    xmlParseComment(parserContext);
		    if (parserContext->instate == XML_PARSER_EOF)
			goto done;
		} else if ((parserContext->instate == XML_PARSER_MISC) &&
                    (cur == '<') && (next == '!') &&
		    (parserContext->input->cur[2] == 'D') &&
		    (parserContext->input->cur[3] == 'O') &&
		    (parserContext->input->cur[4] == 'C') &&
		    (parserContext->input->cur[5] == 'T') &&
		    (parserContext->input->cur[6] == 'Y') &&
		    (parserContext->input->cur[7] == 'P') &&
		    (parserContext->input->cur[8] == 'E')) {
		    if ((!terminate) && (!xmlParseLookupGt(parserContext)))
                        goto done;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: Parsing internal subset\n");
#endif
		    parserContext->inSubset = 1;
		    xmlParseDocTypeDecl(parserContext);
		    if (parserContext->instate == XML_PARSER_EOF)
			goto done;
		    if (RAW == '[') {
			parserContext->instate = XML_PARSER_DTD;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering DTD\n");
#endif
		    } else {
			/*
			 * Create and update the external subset.
			 */
			parserContext->inSubset = 2;
			if ((parserContext->sax != NULL) && (!parserContext->disableSAX) &&
			    (parserContext->sax->externalSubset != NULL))
			    parserContext->sax->externalSubset(parserContext->userData,
				    parserContext->intSubName, parserContext->extSubSystem,
				    parserContext->extSubURI);
			parserContext->inSubset = 0;
			xmlCleanSpecialAttr(parserContext);
			parserContext->instate = XML_PARSER_PROLOG;
#ifdef DEBUG_PUSH
			xmlGenericError(xmlGenericErrorContext,
				"PP: entering PROLOG\n");
#endif
		    }
		} else if ((cur == '<') && (next == '!') &&
		           (avail <
                            (parserContext->instate == XML_PARSER_MISC ? 9 : 4))) {
		    goto done;
		} else if (parserContext->instate == XML_PARSER_EPILOG) {
		    xmlFatalErr(parserContext, XML_ERR_DOCUMENT_END, NULL);
		    xmlHaltParser(parserContext);
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering EOF\n");
#endif
		    if ((parserContext->sax) && (parserContext->sax->endDocument != NULL))
			parserContext->sax->endDocument(parserContext->userData);
		    goto done;
                } else {
		    parserContext->instate = XML_PARSER_START_TAG;
#ifdef DEBUG_PUSH
		    xmlGenericError(xmlGenericErrorContext,
			    "PP: entering START_TAG\n");
#endif
		}
		break;
            case XML_PARSER_DTD: {
                if ((!terminate) && (!xmlParseLookupInternalSubset(parserContext)))
                    goto done;
		xmlParseInternalSubset(parserContext);
		if (parserContext->instate == XML_PARSER_EOF)
		    goto done;
		parserContext->inSubset = 2;
		if ((parserContext->sax != NULL) && (!parserContext->disableSAX) &&
		    (parserContext->sax->externalSubset != NULL))
		    parserContext->sax->externalSubset(parserContext->userData, parserContext->intSubName,
			    parserContext->extSubSystem, parserContext->extSubURI);
		parserContext->inSubset = 0;
		xmlCleanSpecialAttr(parserContext);
		if (parserContext->instate == XML_PARSER_EOF)
		    goto done;
		parserContext->instate = XML_PARSER_PROLOG;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering PROLOG\n");
#endif
                break;
	    }
            case XML_PARSER_COMMENT:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == COMMENT\n");
		parserContext->instate = XML_PARSER_CONTENT;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering CONTENT\n");
#endif
		break;
            case XML_PARSER_IGNORE:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == IGNORE");
	        parserContext->instate = XML_PARSER_DTD;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering DTD\n");
#endif
	        break;
            case XML_PARSER_PI:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == PI\n");
		parserContext->instate = XML_PARSER_CONTENT;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering CONTENT\n");
#endif
		break;
            case XML_PARSER_ENTITY_DECL:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == ENTITY_DECL\n");
		parserContext->instate = XML_PARSER_DTD;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering DTD\n");
#endif
		break;
            case XML_PARSER_ENTITY_VALUE:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == ENTITY_VALUE\n");
		parserContext->instate = XML_PARSER_CONTENT;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering DTD\n");
#endif
		break;
            case XML_PARSER_ATTRIBUTE_VALUE:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == ATTRIBUTE_VALUE\n");
		parserContext->instate = XML_PARSER_START_TAG;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering START_TAG\n");
#endif
		break;
            case XML_PARSER_SYSTEM_LITERAL:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == SYSTEM_LITERAL\n");
		parserContext->instate = XML_PARSER_START_TAG;
#ifdef DEBUG_PUSH
		xmlGenericError(xmlGenericErrorContext,
			"PP: entering START_TAG\n");
#endif
		break;
            case XML_PARSER_PUBLIC_LITERAL:
		xmlGenericError(xmlGenericErrorContext,
			"PP: internal error, state == PUBLIC_LITERAL\n");
		parserContext->instate = XML_PARSER_START_TAG;
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