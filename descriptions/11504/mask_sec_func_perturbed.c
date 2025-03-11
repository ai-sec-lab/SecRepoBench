void
xmlSAX2StartElementNs(void *ctx,
                      const xmlChar *localname,
		      const xmlChar *prefix,
		      const xmlChar *URI,
		      int nb_namespaces,
		      const xmlChar **namespaces,
		      int nb_attributes,
		      int nb_defaulted,
		      const xmlChar **attributes)
{
    xmlParserCtxtPtr parser_ctxt = (xmlParserCtxtPtr) ctx;
    xmlNodePtr ret;
    xmlNodePtr parent;
    xmlNsPtr last = NULL, ns;
    const xmlChar *uri, *pref;
    xmlChar *lname = NULL;
    int i, j;

    if (ctx == NULL) return;
    parent = parser_ctxt->node;
    /*
     * First check on validity:
     */
    if (parser_ctxt->validate && (parser_ctxt->myDoc->extSubset == NULL) &&
        ((parser_ctxt->myDoc->intSubset == NULL) ||
	 ((parser_ctxt->myDoc->intSubset->notations == NULL) &&
	  (parser_ctxt->myDoc->intSubset->elements == NULL) &&
	  (parser_ctxt->myDoc->intSubset->attributes == NULL) &&
	  (parser_ctxt->myDoc->intSubset->entities == NULL)))) {
	xmlErrValid(parser_ctxt, XML_DTD_NO_DTD,
	  "Validation failed: no DTD found !", NULL, NULL);
	parser_ctxt->validate = 0;
    }

    /*
     * Take care of the rare case of an undefined namespace prefix
     */
    if ((prefix != NULL) && (URI == NULL)) {
        if (parser_ctxt->dictNames) {
	    const xmlChar *fullname;

	    fullname = xmlDictQLookup(parser_ctxt->dict, prefix, localname);
	    if (fullname != NULL)
	        localname = fullname;
	} else {
	    lname = xmlBuildQName(localname, prefix, NULL, 0);
	}
    }
    /*
     * allocate the node
     */
    if (parser_ctxt->freeElems != NULL) {
        // <MASK>
	if ((__xmlRegisterCallbacks) && (xmlRegisterNodeDefaultValue))
	    xmlRegisterNodeDefaultValue(ret);
    } else {
	if (parser_ctxt->dictNames)
	    ret = xmlNewDocNodeEatName(parser_ctxt->myDoc, NULL,
	                               (xmlChar *) localname, NULL);
	else if (lname == NULL)
	    ret = xmlNewDocNode(parser_ctxt->myDoc, NULL, localname, NULL);
	else
	    ret = xmlNewDocNodeEatName(parser_ctxt->myDoc, NULL,
	                               (xmlChar *) lname, NULL);
	if (ret == NULL) {
	    xmlSAX2ErrMemory(parser_ctxt, "xmlSAX2StartElementNs");
	    return;
	}
    }
    if (parser_ctxt->linenumbers) {
	if (parser_ctxt->input != NULL) {
	    if (parser_ctxt->input->line < 65535)
		ret->line = (short) parser_ctxt->input->line;
	    else
	        ret->line = 65535;
	}
    }

    if (parent == NULL) {
        xmlAddChild((xmlNodePtr) parser_ctxt->myDoc, (xmlNodePtr) ret);
    }
    /*
     * Build the namespace list
     */
    for (i = 0,j = 0;j < nb_namespaces;j++) {
        pref = namespaces[i++];
	uri = namespaces[i++];
	ns = xmlNewNs(NULL, uri, pref);
	if (ns != NULL) {
	    if (last == NULL) {
	        ret->nsDef = last = ns;
	    } else {
	        last->next = ns;
		last = ns;
	    }
	    if ((URI != NULL) && (prefix == pref))
		ret->ns = ns;
	} else {
            /*
             * any out of memory error would already have been raised
             * but we can't be guaranteed it's the actual error due to the
             * API, best is to skip in this case
             */
	    continue;
	}
#ifdef LIBXML_VALID_ENABLED
	if ((!parser_ctxt->html) && parser_ctxt->validate && parser_ctxt->wellFormed &&
	    parser_ctxt->myDoc && parser_ctxt->myDoc->intSubset) {
	    parser_ctxt->valid &= xmlValidateOneNamespace(&parser_ctxt->vctxt, parser_ctxt->myDoc,
	                                           ret, prefix, ns, uri);
	}
#endif /* LIBXML_VALID_ENABLED */
    }
    parser_ctxt->nodemem = -1;

    /*
     * We are parsing a new node.
     */
    if (nodePush(parser_ctxt, ret) < 0) {
        xmlUnlinkNode(ret);
        xmlFreeNode(ret);
        return;
    }

    /*
     * Link the child element
     */
    if (parent != NULL) {
        if (parent->type == XML_ELEMENT_NODE) {
	    xmlAddChild(parent, ret);
	} else {
	    xmlAddSibling(parent, ret);
	}
    }

    /*
     * Insert the defaulted attributes from the DTD only if requested:
     */
    if ((nb_defaulted != 0) &&
        ((parser_ctxt->loadsubset & XML_COMPLETE_ATTRS) == 0))
	nb_attributes -= nb_defaulted;

    /*
     * Search the namespace if it wasn't already found
     * Note that, if prefix is NULL, this searches for the default Ns
     */
    if ((URI != NULL) && (ret->ns == NULL)) {
        ret->ns = xmlSearchNs(parser_ctxt->myDoc, parent, prefix);
	if ((ret->ns == NULL) && (xmlStrEqual(prefix, BAD_CAST "xml"))) {
	    ret->ns = xmlSearchNs(parser_ctxt->myDoc, ret, prefix);
	}
	if (ret->ns == NULL) {
	    ns = xmlNewNs(ret, NULL, prefix);
	    if (ns == NULL) {

	        xmlSAX2ErrMemory(parser_ctxt, "xmlSAX2StartElementNs");
		return;
	    }
            if (prefix != NULL)
                xmlNsWarnMsg(parser_ctxt, XML_NS_ERR_UNDEFINED_NAMESPACE,
                             "Namespace prefix %s was not found\n",
                             prefix, NULL);
            else
                xmlNsWarnMsg(parser_ctxt, XML_NS_ERR_UNDEFINED_NAMESPACE,
                             "Namespace default prefix was not found\n",
                             NULL, NULL);
	}
    }

    /*
     * process all the other attributes
     */
    if (nb_attributes > 0) {
        for (j = 0,i = 0;i < nb_attributes;i++,j+=5) {
	    /*
	     * Handle the rare case of an undefined atribute prefix
	     */
	    if ((attributes[j+1] != NULL) && (attributes[j+2] == NULL)) {
		if (parser_ctxt->dictNames) {
		    const xmlChar *fullname;

		    fullname = xmlDictQLookup(parser_ctxt->dict, attributes[j+1],
		                              attributes[j]);
		    if (fullname != NULL) {
			xmlSAX2AttributeNs(parser_ctxt, fullname, NULL,
			                   attributes[j+3], attributes[j+4]);
		        continue;
		    }
		} else {
		    lname = xmlBuildQName(attributes[j], attributes[j+1],
		                          NULL, 0);
		    if (lname != NULL) {
			xmlSAX2AttributeNs(parser_ctxt, lname, NULL,
			                   attributes[j+3], attributes[j+4]);
			xmlFree(lname);
		        continue;
		    }
		}
	    }
	    xmlSAX2AttributeNs(parser_ctxt, attributes[j], attributes[j+1],
			       attributes[j+3], attributes[j+4]);
	}
    }

#ifdef LIBXML_VALID_ENABLED
    /*
     * If it's the Document root, finish the DTD validation and
     * check the document root element for validity
     */
    if ((parser_ctxt->validate) && (parser_ctxt->vctxt.finishDtd == XML_CTXT_FINISH_DTD_0)) {
	int chk;

	chk = xmlValidateDtdFinal(&parser_ctxt->vctxt, parser_ctxt->myDoc);
	if (chk <= 0)
	    parser_ctxt->valid = 0;
	if (chk < 0)
	    parser_ctxt->wellFormed = 0;
	parser_ctxt->valid &= xmlValidateRoot(&parser_ctxt->vctxt, parser_ctxt->myDoc);
	parser_ctxt->vctxt.finishDtd = XML_CTXT_FINISH_DTD_1;
    }
#endif /* LIBXML_VALID_ENABLED */
}