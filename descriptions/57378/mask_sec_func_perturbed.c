void
xsltKeyFunction(xmlXPathParserContextPtr ctxt, int nargs){
    xmlXPathObjectPtr obj1, keyValue;

    if (nargs != 2) {
	xsltTransformError(xsltXPathGetTransformContext(ctxt), NULL, NULL,
		"key() : expects two arguments\n");
	ctxt->error = XPATH_INVALID_ARITY;
	return;
    }

    /*
    * Get the key's value.
    */
    keyValue = valuePop(ctxt);
    xmlXPathStringFunction(ctxt, 1);
    if ((keyValue == NULL) ||
	(ctxt->value == NULL) || (ctxt->value->type != XPATH_STRING)) {
	xsltTransformError(xsltXPathGetTransformContext(ctxt), NULL, NULL,
	    "key() : invalid arg expecting a string\n");
	ctxt->error = XPATH_INVALID_TYPE;
	xmlXPathFreeObject(keyValue);

	return;
    }
    /*
    * Get the key's name.
    */
    obj1 = valuePop(ctxt);

    if ((keyValue->type == XPATH_NODESET) || (keyValue->type == XPATH_XSLT_TREE)) {
	int i;
	xmlXPathObjectPtr newobj, ret;

	ret = xmlXPathNewNodeSet(NULL);
        // <MASK>
    } else {
	xmlNodeSetPtr nodelist = NULL;
	xmlChar *key = NULL, *value;
	const xmlChar *keyURI;
	xsltTransformContextPtr tctxt;
	xmlChar *qname, *prefix;
	xmlXPathContextPtr xpctxt = ctxt->context;
	xmlNodePtr tmpNode = NULL;
	xsltDocumentPtr oldDocInfo;

	tctxt = xsltXPathGetTransformContext(ctxt);

	oldDocInfo = tctxt->document;

	if (xpctxt->node == NULL) {
	    xsltTransformError(tctxt, NULL, tctxt->inst,
		"Internal error in xsltKeyFunction(): "
		"The context node is not set on the XPath context.\n");
	    tctxt->state = XSLT_STATE_STOPPED;
	    goto error;
	}
	/*
	 * Get the associated namespace URI if qualified name
	 */
	qname = obj1->stringval;
	key = xmlSplitQName2(qname, &prefix);
	if (key == NULL) {
	    key = xmlStrdup(obj1->stringval);
	    keyURI = NULL;
	    if (prefix != NULL)
		xmlFree(prefix);
	} else {
	    if (prefix != NULL) {
		keyURI = xmlXPathNsLookup(xpctxt, prefix);
		if (keyURI == NULL) {
		    xsltTransformError(tctxt, NULL, tctxt->inst,
			"key() : prefix %s is not bound\n", prefix);
		    /*
		    * TODO: Shouldn't we stop here?
		    */
		}
		xmlFree(prefix);
	    } else {
		keyURI = NULL;
	    }
	}

	/*
	 * Force conversion of first arg to string
	 */
	valuePush(ctxt, keyValue);
	xmlXPathStringFunction(ctxt, 1);
	keyValue = valuePop(ctxt);
	if ((keyValue == NULL) || (keyValue->type != XPATH_STRING)) {
	    xsltTransformError(tctxt, NULL, tctxt->inst,
		"key() : invalid arg expecting a string\n");
	    ctxt->error = XPATH_INVALID_TYPE;
	    goto error;
	}
	value = keyValue->stringval;

	/*
	* We need to ensure that ctxt->document is available for
	* xsltGetKey().
	* First find the relevant doc, which is the context node's
	* owner doc; using context->doc is not safe, since
	* the doc could have been acquired via the document() function,
	* or the doc might be a Result Tree Fragment.
	* FUTURE INFO: In XSLT 2.0 the key() function takes an additional
	* argument indicating the doc to use.
	*/
	if (xpctxt->node->type == XML_NAMESPACE_DECL) {
	    /*
	    * REVISIT: This is a libxml hack! Check xpath.c for details.
	    * The XPath module sets the owner element of a ns-node on
	    * the ns->next field.
	    */
	    if ((((xmlNsPtr) xpctxt->node)->next != NULL) &&
		(((xmlNsPtr) xpctxt->node)->next->type == XML_ELEMENT_NODE))
	    {
		tmpNode = (xmlNodePtr) ((xmlNsPtr) xpctxt->node)->next;
	    }
	} else
	    tmpNode = xpctxt->node;

	if ((tmpNode == NULL) || (tmpNode->doc == NULL)) {
	    xsltTransformError(tctxt, NULL, tctxt->inst,
		"Internal error in xsltKeyFunction(): "
		"Couldn't get the doc of the XPath context node.\n");
	    goto error;
	}

	if ((tctxt->document == NULL) ||
	    (tctxt->document->doc != tmpNode->doc))
	{
	    if (tmpNode->doc->name && (tmpNode->doc->name[0] == ' ')) {
		/*
		* This is a Result Tree Fragment.
		*/
		if (tmpNode->doc->_private == NULL) {
		    tmpNode->doc->_private = xsltNewDocument(tctxt, tmpNode->doc);
		    if (tmpNode->doc->_private == NULL)
			goto error;
		}
		tctxt->document = (xsltDocumentPtr) tmpNode->doc->_private;
	    } else {
		/*
		* May be the initial source doc or a doc acquired via the
		* document() function.
		*/
		tctxt->document = xsltFindDocument(tctxt, tmpNode->doc);
	    }
	    if (tctxt->document == NULL) {
		xsltTransformError(tctxt, NULL, tctxt->inst,
		    "Internal error in xsltKeyFunction(): "
		    "Could not get the document info of a context doc.\n");
		tctxt->state = XSLT_STATE_STOPPED;
		goto error;
	    }
	}
	/*
	* Get/compute the key value.
	*/
	nodelist = xsltGetKey(tctxt, key, keyURI, value);

error:
	tctxt->document = oldDocInfo;
	valuePush(ctxt, xmlXPathWrapNodeSet(
	    xmlXPathNodeSetMerge(NULL, nodelist)));
	if (key != NULL)
	    xmlFree(key);
    }

    if (obj1 != NULL)
	xmlXPathFreeObject(obj1);
    if (keyValue != NULL)
	xmlXPathFreeObject(keyValue);
}