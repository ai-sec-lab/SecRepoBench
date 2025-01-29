while ((cur->type != XML_DTD_NODE) &&
               (cur->type != XML_ENTITY_REF_NODE) &&
               (cur->children != NULL) &&
               (cur->children->parent == cur)) {
            cur = cur->children;
            depth += 1;
        }

        next = cur->next;
        parent = cur->parent;

	/* unroll to speed up freeing the document */
	if (cur->type != XML_DTD_NODE) {

	    if ((__xmlRegisterCallbacks) && (xmlDeregisterNodeDefaultValue))
		xmlDeregisterNodeDefaultValue(cur);

	    if (((cur->type == XML_ELEMENT_NODE) ||
		 (cur->type == XML_XINCLUDE_START) ||
		 (cur->type == XML_XINCLUDE_END)) &&
		(cur->properties != NULL))
		xmlTextReaderFreePropList(reader, cur->properties);
	    if ((cur->content != (xmlChar *) &(cur->properties)) &&
	        (cur->type != XML_ELEMENT_NODE) &&
		(cur->type != XML_XINCLUDE_START) &&
		(cur->type != XML_XINCLUDE_END) &&
		(cur->type != XML_ENTITY_REF_NODE)) {
		DICT_FREE(cur->content);
	    }
	    if (((cur->type == XML_ELEMENT_NODE) ||
	         (cur->type == XML_XINCLUDE_START) ||
		 (cur->type == XML_XINCLUDE_END)) &&
		(cur->nsDef != NULL))
		xmlFreeNsList(cur->nsDef);

	    /*
	     * we don't free element names here they are interned now
	     */
	    if ((cur->type != XML_TEXT_NODE) &&
		(cur->type != XML_COMMENT_NODE))
		DICT_FREE(cur->name);
	    if (((cur->type == XML_ELEMENT_NODE) ||
		 (cur->type == XML_TEXT_NODE)) &&
	        (reader != NULL) && (reader->ctxt != NULL) &&
		(reader->ctxt->freeElemsNr < 100)) {
	        cur->next = reader->ctxt->freeElems;
		reader->ctxt->freeElems = cur;
		reader->ctxt->freeElemsNr++;
	    } else {
		xmlFree(cur);
	    }
	}

        if (next != NULL) {
	    cur = next;
        } else {
            if ((depth == 0) || (parent == NULL))
                break;
            depth -= 1;
            cur = parent;
            cur->children = NULL;
        }