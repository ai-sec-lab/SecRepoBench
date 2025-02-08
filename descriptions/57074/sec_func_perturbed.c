static void
xsltCleanupSourceDoc(xmlDocPtr sourceDoc) {
    xmlNodePtr cur = (xmlNodePtr) sourceDoc;
    void **psviPtr;

    while (1) {
        xsltClearSourceNodeFlags(cur, XSLT_SOURCE_NODE_MASK);
        psviPtr = xsltGetPSVIPtr(cur);
        if (psviPtr)
            *psviPtr = NULL;

        if (cur->type == XML_ELEMENT_NODE) {
            xmlAttrPtr prop = cur->properties;

            while (prop) {
                prop->atype &= ~(XSLT_SOURCE_NODE_MASK << 27);
                prop->psvi = NULL;
                prop = prop->next;
            }
        }

        if (cur->children != NULL && cur->type != XML_ENTITY_REF_NODE) {
            cur = cur->children;
        } else {
            if (cur == (xmlNodePtr) sourceDoc)
                return;
            while (cur->next == NULL) {
                cur = cur->parent;
                if (cur == (xmlNodePtr) sourceDoc)
                    return;
            }

            cur = cur->next;
        }
    }
}