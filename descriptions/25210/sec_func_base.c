int
xmlTextReaderRead(xmlTextReaderPtr reader) {
    int val, olddepth = 0;
    xmlTextReaderState oldstate = XML_TEXTREADER_START;
    xmlNodePtr oldnode = NULL;


    if (reader == NULL)
	return(-1);
    reader->curnode = NULL;
    if (reader->doc != NULL)
        return(xmlTextReaderReadTree(reader));
    if (reader->ctxt == NULL)
	return(-1);

#ifdef DEBUG_READER
    fprintf(stderr, "\nREAD ");
    DUMP_READER
#endif
    if (reader->mode == XML_TEXTREADER_MODE_INITIAL) {
	reader->mode = XML_TEXTREADER_MODE_INTERACTIVE;
	/*
	 * Initial state
	 */
	do {
	    val = xmlTextReaderPushData(reader);
		if (val < 0){
			reader->mode = XML_TEXTREADER_MODE_ERROR;
			reader->state = XML_TEXTREADER_ERROR;
		return(-1);
		}
	} while ((reader->ctxt->node == NULL) &&
		 ((reader->mode != XML_TEXTREADER_MODE_EOF) &&
		  (reader->state != XML_TEXTREADER_DONE)));
	if (reader->ctxt->node == NULL) {
	    if (reader->ctxt->myDoc != NULL) {
		reader->node = reader->ctxt->myDoc->children;
	    }
	    if (reader->node == NULL){
			reader->mode = XML_TEXTREADER_MODE_ERROR;
			reader->state = XML_TEXTREADER_ERROR;
		return(-1);
		}
	    reader->state = XML_TEXTREADER_ELEMENT;
	} else {
	    if (reader->ctxt->myDoc != NULL) {
		reader->node = reader->ctxt->myDoc->children;
	    }
	    if (reader->node == NULL)
		reader->node = reader->ctxt->nodeTab[0];
	    reader->state = XML_TEXTREADER_ELEMENT;
	}
	reader->depth = 0;
	reader->ctxt->parseMode = XML_PARSE_READER;
	goto node_found;
    }
    oldstate = reader->state;
    olddepth = reader->ctxt->nodeNr;
    oldnode = reader->node;

get_next_node:
    if (reader->node == NULL) {
	if (reader->mode == XML_TEXTREADER_MODE_EOF)
	    return(0);
	else
	    return(-1);
    }

    /*
     * If we are not backtracking on ancestors or examined nodes,
     * that the parser didn't finished or that we aren't at the end
     * of stream, continue processing.
     */
    while ((reader->node != NULL) && (reader->node->next == NULL) &&
	   (reader->ctxt->nodeNr == olddepth) &&
           ((oldstate == XML_TEXTREADER_BACKTRACK) ||
            (reader->node->children == NULL) ||
	    (reader->node->type == XML_ENTITY_REF_NODE) ||
	    ((reader->node->children != NULL) &&
	     (reader->node->children->type == XML_TEXT_NODE) &&
	     (reader->node->children->next == NULL)) ||
	    (reader->node->type == XML_DTD_NODE) ||
	    (reader->node->type == XML_DOCUMENT_NODE) ||
	    (reader->node->type == XML_HTML_DOCUMENT_NODE)) &&
	   ((reader->ctxt->node == NULL) ||
	    (reader->ctxt->node == reader->node) ||
	    (reader->ctxt->node == reader->node->parent)) &&
	   (reader->ctxt->instate != XML_PARSER_EOF)) {
	val = xmlTextReaderPushData(reader);
	if (val < 0){
		reader->mode = XML_TEXTREADER_MODE_ERROR;
		reader->state = XML_TEXTREADER_ERROR;
	    return(-1);
	}
	if (reader->node == NULL)
	    goto node_end;
    }
    if (oldstate != XML_TEXTREADER_BACKTRACK) {
	if ((reader->node->children != NULL) &&
	    (reader->node->type != XML_ENTITY_REF_NODE) &&
	    (reader->node->type != XML_XINCLUDE_START) &&
	    (reader->node->type != XML_DTD_NODE)) {
	    reader->node = reader->node->children;
	    reader->depth++;
	    reader->state = XML_TEXTREADER_ELEMENT;
	    goto node_found;
	}
    }
    if (reader->node->next != NULL) {
	if ((oldstate == XML_TEXTREADER_ELEMENT) &&
            (reader->node->type == XML_ELEMENT_NODE) &&
	    (reader->node->children == NULL) &&
	    ((reader->node->extra & NODE_IS_EMPTY) == 0)
#ifdef LIBXML_XINCLUDE_ENABLED
	    && (reader->in_xinclude <= 0)
#endif
	    ) {
	    reader->state = XML_TEXTREADER_END;
	    goto node_found;
	}
#ifdef LIBXML_REGEXP_ENABLED
	if ((reader->validate) &&
	    (reader->node->type == XML_ELEMENT_NODE))
	    xmlTextReaderValidatePop(reader);
#endif /* LIBXML_REGEXP_ENABLED */
        if ((reader->preserves > 0) &&
	    (reader->node->extra & NODE_IS_SPRESERVED))
	    reader->preserves--;
	reader->node = reader->node->next;
	reader->state = XML_TEXTREADER_ELEMENT;

	/*
	 * Cleanup of the old node
	 */
	if ((reader->preserves == 0) &&
#ifdef LIBXML_XINCLUDE_ENABLED
	    (reader->in_xinclude == 0) &&
#endif
	    (reader->entNr == 0) &&
	    (reader->node->prev != NULL) &&
            (reader->node->prev->type != XML_DTD_NODE)) {
	    xmlNodePtr tmp = reader->node->prev;
	    if ((tmp->extra & NODE_IS_PRESERVED) == 0) {
                if (oldnode == tmp)
                    oldnode = NULL;
		xmlUnlinkNode(tmp);
		xmlTextReaderFreeNode(reader, tmp);
	    }
	}

	goto node_found;
    }
    if ((oldstate == XML_TEXTREADER_ELEMENT) &&
	(reader->node->type == XML_ELEMENT_NODE) &&
	(reader->node->children == NULL) &&
	((reader->node->extra & NODE_IS_EMPTY) == 0)) {;
	reader->state = XML_TEXTREADER_END;
	goto node_found;
    }
#ifdef LIBXML_REGEXP_ENABLED
    if ((reader->validate != XML_TEXTREADER_NOT_VALIDATE) && (reader->node->type == XML_ELEMENT_NODE))
	xmlTextReaderValidatePop(reader);
#endif /* LIBXML_REGEXP_ENABLED */
    if ((reader->preserves > 0) &&
	(reader->node->extra & NODE_IS_SPRESERVED))
	reader->preserves--;
    reader->node = reader->node->parent;
    if ((reader->node == NULL) ||
	(reader->node->type == XML_DOCUMENT_NODE) ||
#ifdef LIBXML_DOCB_ENABLED
	(reader->node->type == XML_DOCB_DOCUMENT_NODE) ||
#endif
	(reader->node->type == XML_HTML_DOCUMENT_NODE)) {
	if (reader->mode != XML_TEXTREADER_MODE_EOF) {
	    val = xmlParseChunk(reader->ctxt, "", 0, 1);
	    reader->state = XML_TEXTREADER_DONE;
	    if (val != 0)
	        return(-1);
	}
	reader->node = NULL;
	reader->depth = -1;

	/*
	 * Cleanup of the old node
	 */
	if ((oldnode != NULL) && (reader->preserves == 0) &&
#ifdef LIBXML_XINCLUDE_ENABLED
	    (reader->in_xinclude == 0) &&
#endif
	    (reader->entNr == 0) &&
	    (oldnode->type != XML_DTD_NODE) &&
	    ((oldnode->extra & NODE_IS_PRESERVED) == 0)) {
	    xmlUnlinkNode(oldnode);
	    xmlTextReaderFreeNode(reader, oldnode);
	}

	goto node_end;
    }
    if ((reader->preserves == 0) &&
#ifdef LIBXML_XINCLUDE_ENABLED
        (reader->in_xinclude == 0) &&
#endif
	(reader->entNr == 0) &&
        (reader->node->last != NULL) &&
        ((reader->node->last->extra & NODE_IS_PRESERVED) == 0)) {
	xmlNodePtr tmp = reader->node->last;
	xmlUnlinkNode(tmp);
	xmlTextReaderFreeNode(reader, tmp);
    }
    reader->depth--;
    reader->state = XML_TEXTREADER_BACKTRACK;

node_found:
    DUMP_READER

    /*
     * If we are in the middle of a piece of CDATA make sure it's finished
     */
    if ((reader->node != NULL) &&
        (reader->node->next == NULL) &&
        ((reader->node->type == XML_TEXT_NODE) ||
	 (reader->node->type == XML_CDATA_SECTION_NODE))) {
            if (xmlTextReaderExpand(reader) == NULL)
	        return -1;
    }

#ifdef LIBXML_XINCLUDE_ENABLED
    /*
     * Handle XInclude if asked for
     */
    if ((reader->xinclude) && (reader->node != NULL) &&
	(reader->node->type == XML_ELEMENT_NODE) &&
	(reader->node->ns != NULL) &&
	((xmlStrEqual(reader->node->ns->href, XINCLUDE_NS)) ||
	 (xmlStrEqual(reader->node->ns->href, XINCLUDE_OLD_NS)))) {
	if (reader->xincctxt == NULL) {
	    reader->xincctxt = xmlXIncludeNewContext(reader->ctxt->myDoc);
	    xmlXIncludeSetFlags(reader->xincctxt,
	                        reader->parserFlags & (~XML_PARSE_NOXINCNODE));
	}