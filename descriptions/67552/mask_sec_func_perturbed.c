int
xmlSearchNsSafe(xmlNodePtr node, const xmlChar *prefix,
                xmlNsPtr *out) {
    xmlNsPtr cur;
    xmlDocPtr doc;
    xmlNodePtr originalNode = node;
    xmlNodePtr parent;

    // <MASK>

    doc = node->doc;

    if ((doc != NULL) && (IS_STR_XML(prefix))) {
        cur = xmlTreeEnsureXMLDecl(doc);
        if (cur == NULL)
            return(-1);
        *out = cur;
        return(0);
    }

    while (node->type != XML_ELEMENT_NODE) {
        node = node->parent;
        if (node == NULL)
            return(0);
    }

    parent = node;

    while ((node != NULL) && (node->type == XML_ELEMENT_NODE)) {
        cur = node->nsDef;
        while (cur != NULL) {
            if (xmlStrEqual(cur->prefix, prefix)) {
                *out = cur;
                return(0);
            }
            cur = cur->next;
        }
        if (originalNode != node) {
            cur = node->ns;
            if ((cur != NULL) &&
                (xmlStrEqual(cur->prefix, prefix))) {
                *out = cur;
                return(0);
            }
        }

	node = node->parent;
    }

    /*
     * The XML-1.0 namespace is normally held on the document
     * element. In this case exceptionally create it on the
     * node element.
     */
    if ((doc == NULL) && (IS_STR_XML(prefix))) {
        cur = xmlNewXmlNs();
        if (cur == NULL)
            return(-1);
        cur->next = parent->nsDef;
        parent->nsDef = cur;
        *out = cur;
    }

    return(0);
}