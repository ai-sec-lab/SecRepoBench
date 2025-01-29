ctxt->freeElems = ret->next;
	ctxt->freeElemsNr--;
	memset(ret, 0, sizeof(xmlNode));
        ret->doc = ctxt->myDoc;
	ret->type = XML_ELEMENT_NODE;

	if (ctxt->dictNames)
	    ret->name = localname;
	else {
	    if (lname == NULL)
		ret->name = xmlStrdup(localname);
	    else
	        ret->name = lname;
	    if (ret->name == NULL) {
	        xmlSAX2ErrMemory(ctxt, "xmlSAX2StartElementNs");
		return;
	    }
	}