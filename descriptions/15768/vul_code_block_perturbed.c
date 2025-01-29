{
	/*
	* TODO: DO we want to use this also for "text" output?
	*/
        copy = xmlNewTextLen(NULL, 0);
	if (copy == NULL)
	    goto exit;
	if (currentNode->name == xmlStringTextNoenc)
	    copy->name = xmlStringTextNoenc;

	/*
	 * Must confirm that content is in dict (bug 302821)
	 * TODO: This check should be not needed for text coming
	 * from the stylesheets
	 */
	if (xmlDictOwns(ctxt->dict, currentNode->content))
	    copy->content = currentNode->content;
	else {
	    if ((copy->content = xmlStrdup(currentNode->content)) == NULL)
		return NULL;
	}
    } else {
        /*
	 * normal processing. keep counters to extend the text node
	 * in xsltAddTextString if needed.
	 */
        unsigned int len;

	len = xmlStrlen(currentNode->content);
	copy = xmlNewTextLen(currentNode->content, len);
	if (copy == NULL)
	    goto exit;
	if (currentNode->name == xmlStringTextNoenc)
	    copy->name = xmlStringTextNoenc;
	ctxt->lasttext = copy->content;
	ctxt->lasttsize = len;
	ctxt->lasttuse = len;
    }