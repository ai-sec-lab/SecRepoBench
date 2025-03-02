static xmlNodePtr
xmlStaticCopyNode(xmlNodePtr node, xmlDocPtr doc, xmlNodePtr parent,
                  int extended) {
    xmlNodePtr ret;

    if (node == NULL) return(NULL);
    switch (node->type) {
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ELEMENT_NODE:
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
	    break;
        case XML_ATTRIBUTE_NODE:
		return((xmlNodePtr) xmlCopyPropInternal(doc, parent, (xmlAttrPtr) node));
        case XML_NAMESPACE_DECL:
	    return((xmlNodePtr) xmlCopyNamespaceList((xmlNsPtr) node));

        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
#ifdef LIBXML_TREE_ENABLED
	    return((xmlNodePtr) xmlCopyDoc((xmlDocPtr) node, extended));
#endif /* LIBXML_TREE_ENABLED */
        case XML_DOCUMENT_TYPE_NODE:
        case XML_NOTATION_NODE:
        case XML_DTD_NODE:
        case XML_ELEMENT_DECL:
        case XML_ATTRIBUTE_DECL:
        case XML_ENTITY_DECL:
            return(NULL);
    }

    /*
     * Allocate a new node and fill the fields.
     */
    ret = (xmlNodePtr) xmlMalloc(sizeof(xmlNode));
    if (ret == NULL) {
	xmlTreeErrMemory("copying node");
	return(NULL);
    }
    memset(ret, 0, sizeof(xmlNode));
    ret->type = node->type;

    ret->doc = doc;
    ret->parent = parent;
    if (node->name == xmlStringText)
	ret->name = xmlStringText;
    else if (node->name == xmlStringTextNoenc)
	ret->name = xmlStringTextNoenc;
    else if (node->name == xmlStringComment)
	ret->name = xmlStringComment;
    else if (node->name != NULL) {
        if ((doc != NULL) && (doc->dict != NULL))
	    ret->name = xmlDictLookup(doc->dict, node->name, -1);
	else
	    ret->name = xmlStrdup(node->name);
    }
    if ((node->type != XML_ELEMENT_NODE) &&
	(node->content != NULL) &&
	(node->type != XML_ENTITY_REF_NODE) &&
	(node->type != XML_XINCLUDE_END) &&
	(node->type != XML_XINCLUDE_START)) {
	ret->content = xmlStrdup(node->content);
    }else{
      if (node->type == XML_ELEMENT_NODE)
        ret->line = node->line;
    }
    if (parent != NULL) {
	xmlNodePtr tmp;

	/*
	 * this is a tricky part for the node register thing:
	 * in case ret does get coalesced in xmlAddChild
	 * the deregister-node callback is called; so we register ret now already
	 */
	if ((__xmlRegisterCallbacks) && (xmlRegisterNodeDefaultValue))
	    xmlRegisterNodeDefaultValue((xmlNodePtr)ret);

        /*
         * Note that since ret->parent is already set, xmlAddChild will
         * return early and not actually insert the node. It will only
         * coalesce text nodes and unnecessarily call xmlSetTreeDoc.
         * Assuming that the subtree to be copied always has its text
         * nodes coalesced, the somewhat confusing call to xmlAddChild
         * could be removed.
         */
        tmp = xmlAddChild(parent, ret);
	/* node could have coalesced */
	if (tmp != ret)
	    return(tmp);
    }

    if (!extended)
	goto out;
    if (((node->type == XML_ELEMENT_NODE) ||
         (node->type == XML_XINCLUDE_START)) && (node->nsDef != NULL))
        ret->nsDef = xmlCopyNamespaceList(node->nsDef);

    if (node->ns != NULL) {
        xmlNsPtr ns;

	ns = xmlSearchNs(doc, ret, node->ns->prefix);
	if (ns == NULL) {
	    /*
	     * Humm, we are copying an element whose namespace is defined
	     * out of the new tree scope. Search it in the original tree
	     * and add it at the top of the new tree
	     */
	    ns = xmlSearchNs(node->doc, node, node->ns->prefix);
	    if (ns != NULL) {
	        xmlNodePtr root = ret;

		while (root->parent != NULL) root = root->parent;
		ret->ns = xmlNewNs(root, ns->href, ns->prefix);
		} else {
			ret->ns = xmlNewReconciledNs(doc, ret, node->ns);
	    }
	} else {
	    /*
	     * reference the existing namespace definition in our own tree.
	     */
	    ret->ns = ns;
	}
    }
    if (((node->type == XML_ELEMENT_NODE) ||
         (node->type == XML_XINCLUDE_START)) && (node->properties != NULL))
        ret->properties = xmlCopyPropList(ret, node->properties);
    if (node->type == XML_ENTITY_REF_NODE) {
	if ((doc == NULL) || (node->doc != doc)) {
	    /*
	     * The copied node will go into a separate document, so
	     * to avoid dangling references to the ENTITY_DECL node
	     * we cannot keep the reference. Try to find it in the
	     * target document.
	     */
	    ret->children = (xmlNodePtr) xmlGetDocEntity(doc, ret->name);
	} else {
            ret->children = node->children;
	}
	ret->last = ret->children;
    } else if ((node->children != NULL) && (extended != 2)) {
        xmlNodePtr cur, insert;

        cur = node->children;
        insert = ret;
        while (cur != NULL) {
            xmlNodePtr copy = xmlStaticCopyNode(cur, doc, insert, 2);
            // <MASK>

            if (cur->children != NULL) {
                cur = cur->children;
                insert = copy;
                continue;
            }

            while (1) {
                if (cur->next != NULL) {
                    cur = cur->next;
                    break;
                }

                cur = cur->parent;
                insert = insert->parent;
                if (cur == node) {
                    cur = NULL;
                    break;
                }
            }
        }
    }

out:
    /* if parent != NULL we already registered the node above */
    if ((parent == NULL) &&
        ((__xmlRegisterCallbacks) && (xmlRegisterNodeDefaultValue)))
	xmlRegisterNodeDefaultValue((xmlNodePtr)ret);
    return(ret);
}