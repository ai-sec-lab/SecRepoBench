static xmlNodePtr
xmlStaticCopyNode(xmlNodePtr node, xmlDocPtr doc, xmlNodePtr parent,
                  int extended) {
    xmlNodePtr copyNode;

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
    copyNode = (xmlNodePtr) xmlMalloc(sizeof(xmlNode));
    if (copyNode == NULL) {
	xmlTreeErrMemory("copying node");
	return(NULL);
    }
    memset(copyNode, 0, sizeof(xmlNode));
    copyNode->type = node->type;

    copyNode->doc = doc;
    copyNode->parent = parent;
    if (node->name == xmlStringText)
	copyNode->name = xmlStringText;
    else if (node->name == xmlStringTextNoenc)
	copyNode->name = xmlStringTextNoenc;
    else if (node->name == xmlStringComment)
	copyNode->name = xmlStringComment;
    else if (node->name != NULL) {
        if ((doc != NULL) && (doc->dict != NULL))
	    copyNode->name = xmlDictLookup(doc->dict, node->name, -1);
	else
	    copyNode->name = xmlStrdup(node->name);
    }
    if ((node->type != XML_ELEMENT_NODE) &&
	(node->content != NULL) &&
	(node->type != XML_ENTITY_REF_NODE) &&
	(node->type != XML_XINCLUDE_END) &&
	(node->type != XML_XINCLUDE_START)) {
	copyNode->content = xmlStrdup(node->content);
    }else{
      if (node->type == XML_ELEMENT_NODE)
        copyNode->line = node->line;
    }
    if (parent != NULL) {
	xmlNodePtr tmp;

	/*
	 * this is a tricky part for the node register thing:
	 * in case ret does get coalesced in xmlAddChild
	 * the deregister-node callback is called; so we register ret now already
	 */
	if ((__xmlRegisterCallbacks) && (xmlRegisterNodeDefaultValue))
	    xmlRegisterNodeDefaultValue((xmlNodePtr)copyNode);

        /*
         * Note that since ret->parent is already set, xmlAddChild will
         * return early and not actually insert the node. It will only
         * coalesce text nodes and unnecessarily call xmlSetTreeDoc.
         * Assuming that the subtree to be copied always has its text
         * nodes coalesced, the somewhat confusing call to xmlAddChild
         * could be removed.
         */
        tmp = xmlAddChild(parent, copyNode);
	/* node could have coalesced */
	if (tmp != copyNode)
	    return(tmp);
    }

    if (!extended)
	goto out;
    if (((node->type == XML_ELEMENT_NODE) ||
         (node->type == XML_XINCLUDE_START)) && (node->nsDef != NULL))
        copyNode->nsDef = xmlCopyNamespaceList(node->nsDef);

    if (node->ns != NULL) {
        xmlNsPtr ns;

	ns = xmlSearchNs(doc, copyNode, node->ns->prefix);
	if (ns == NULL) {
	    /*
	     * Humm, we are copying an element whose namespace is defined
	     * out of the new tree scope. Search it in the original tree
	     * and add it at the top of the new tree
	     */
	    ns = xmlSearchNs(node->doc, node, node->ns->prefix);
	    if (ns != NULL) {
	        xmlNodePtr root = copyNode;

		while (root->parent != NULL) root = root->parent;
		copyNode->ns = xmlNewNs(root, ns->href, ns->prefix);
		} else {
			copyNode->ns = xmlNewReconciledNs(doc, copyNode, node->ns);
	    }
	} else {
	    /*
	     * reference the existing namespace definition in our own tree.
	     */
	    copyNode->ns = ns;
	}
    }
    if (((node->type == XML_ELEMENT_NODE) ||
         (node->type == XML_XINCLUDE_START)) && (node->properties != NULL))
        copyNode->properties = xmlCopyPropList(copyNode, node->properties);
    if (node->type == XML_ENTITY_REF_NODE) {
	if ((doc == NULL) || (node->doc != doc)) {
	    /*
	     * The copied node will go into a separate document, so
	     * to avoid dangling references to the ENTITY_DECL node
	     * we cannot keep the reference. Try to find it in the
	     * target document.
	     */
	    copyNode->children = (xmlNodePtr) xmlGetDocEntity(doc, copyNode->name);
	} else {
            copyNode->children = node->children;
	}
	copyNode->last = copyNode->children;
    } else if ((node->children != NULL) && (extended != 2)) {
        xmlNodePtr cur, insert;

        cur = node->children;
        insert = copyNode;
        while (cur != NULL) {
            xmlNodePtr copy = xmlStaticCopyNode(cur, doc, insert, 2);
            if (copy == NULL) {
                xmlFreeNode(copyNode);
                return(NULL);
            }

            /* Check for coalesced text nodes */
            if (insert->last != copy) {
                if (insert->last == NULL) {
                    insert->children = copy;
                } else {
                    copy->prev = insert->last;
                    insert->last->next = copy;
                }
                insert->last = copy;
            }

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
	xmlRegisterNodeDefaultValue((xmlNodePtr)copyNode);
    return(copyNode);
}