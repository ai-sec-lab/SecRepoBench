static xmlNodePtr
xsltCopyText(xsltTransformContextPtr ctxt, xmlNodePtr target,
	     xmlNodePtr currentNode, int interned)
{
    xmlNodePtr copy;

    if ((currentNode->type != XML_TEXT_NODE) &&
	(currentNode->type != XML_CDATA_SECTION_NODE))
	return(NULL);
    if (currentNode->content == NULL)
	return(NULL);

#ifdef WITH_XSLT_DEBUG_PROCESS
    if (currentNode->type == XML_CDATA_SECTION_NODE) {
	XSLT_TRACE(ctxt,XSLT_TRACE_COPY_TEXT,xsltGenericDebug(xsltGenericDebugContext,
			 "xsltCopyText: copy CDATA text %s\n",
			 currentNode->content));
    } else if (currentNode->name == xmlStringTextNoenc) {
	XSLT_TRACE(ctxt,XSLT_TRACE_COPY_TEXT,xsltGenericDebug(xsltGenericDebugContext,
		     "xsltCopyText: copy unescaped text %s\n",
			 currentNode->content));
    } else {
	XSLT_TRACE(ctxt,XSLT_TRACE_COPY_TEXT,xsltGenericDebug(xsltGenericDebugContext,
			 "xsltCopyText: copy text %s\n",
			 currentNode->content));
    }
#endif

    /*
    * Play save and reset the merging mechanism for every new
    * target node.
    */
    if ((target == NULL) || (target->children == NULL)) {
	ctxt->lasttext = NULL;
    }

    if ((ctxt->style->cdataSection != NULL) &&
	(ctxt->type == XSLT_OUTPUT_XML) &&
	(target != NULL) &&
	(target->type == XML_ELEMENT_NODE) &&
	(((target->ns == NULL) &&
	  (xmlHashLookup2(ctxt->style->cdataSection,
		          target->name, NULL) != NULL)) ||
	 ((target->ns != NULL) &&
	  (xmlHashLookup2(ctxt->style->cdataSection,
	                  target->name, target->ns->href) != NULL))))
    {
	/*
	* Process "cdata-section-elements".
	*/
	/*
	* OPTIMIZE TODO: xsltCopyText() is also used for attribute content.
	*/
	/*
	* TODO: Since this doesn't merge adjacent CDATA-section nodes,
	* we'll get: <![CDATA[x]]><!CDATA[y]]>.
	* TODO: Reported in #321505.
	*/
	if ((target->last != NULL) &&
	     (target->last->type == XML_CDATA_SECTION_NODE))
	{
	    /*
	    * Append to existing CDATA-section node.
	    */
	    copy = xsltAddTextString(ctxt, target->last, currentNode->content,
		xmlStrlen(currentNode->content));
	    goto exit;
	} else {
	    unsigned int len;

	    len = xmlStrlen(currentNode->content);
	    copy = xmlNewCDataBlock(ctxt->output, currentNode->content, len);
	    if (copy == NULL)
		goto exit;
	    ctxt->lasttext = copy->content;
	    ctxt->lasttsize = len;
	    ctxt->lasttuse = len;
	}
    } else if ((target != NULL) &&
	(target->last != NULL) &&
	/* both escaped or both non-escaped text-nodes */
	(((target->last->type == XML_TEXT_NODE) &&
	(target->last->name == currentNode->name)) ||
        /* non-escaped text nodes and CDATA-section nodes */
	(((target->last->type == XML_CDATA_SECTION_NODE) &&
	(currentNode->name == xmlStringTextNoenc)))))
    {
	/*
	 * we are appending to an existing text node
	 */
	copy = xsltAddTextString(ctxt, target->last, currentNode->content,
	    xmlStrlen(currentNode->content));
	goto exit;
    } else if ((interned) && (target != NULL) &&
	(target->doc != NULL) &&
	(target->doc->dict == ctxt->dict))
    // <MASK>
    if (copy != NULL) {
	if (target != NULL) {
	    copy->doc = target->doc;
	    /*
	    * MAYBE TODO: Maybe we should reset the ctxt->lasttext here
	    *  to ensure that the optimized text-merging mechanism
	    *  won't interfere with normal node-merging in any case.
	    */
	    copy = xsltAddChild(target, copy);
	}
    } else {
	xsltTransformError(ctxt, NULL, target,
			 "xsltCopyText: text copy failed\n");
    }

exit:
    if ((copy == NULL) || (copy->content == NULL)) {
	xsltTransformError(ctxt, NULL, target,
	    "Internal error in xsltCopyText(): "
	    "Failed to copy the string.\n");
	ctxt->state = XSLT_STATE_STOPPED;
    }
    return(copy);
}