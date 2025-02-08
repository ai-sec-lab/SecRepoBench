{
	/*
	* TODO: DO we want to use this also for "text" output?
	*/
        copy = xmlNewTextLen(NULL, 0);
	if (copy == NULL)
	    goto exit;
	if (cur->name == xmlStringTextNoenc)
	    copy->name = xmlStringTextNoenc;

	/*
	 * Must confirm that content is in dict (bug 302821)
	 * TODO: This check should be not needed for text coming
	 * from the stylesheets
	 */
	if (xmlDictOwns(ctxt->dict, cur->content))
	    copy->content = cur->content;
	else {
	    if ((copy->content = xmlStrdup(cur->content)) == NULL)
		return NULL;
	}

	ctxt->lasttext = NULL;
    } else {
        /*
	 * normal processing. keep counters to extend the text node
	 * in xsltAddTextString if needed.
	 */
        unsigned int len;

	len = xmlStrlen(cur->content);
	copy = xmlNewTextLen(cur->content, len);
	if (copy == NULL)
	    goto exit;
	if (cur->name == xmlStringTextNoenc)
	    copy->name = xmlStringTextNoenc;
	ctxt->lasttext = copy->content;
	ctxt->lasttsize = len;
	ctxt->lasttuse = len;
    }