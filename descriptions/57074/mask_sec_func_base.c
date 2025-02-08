static void
xsltCleanupSourceDoc(xmlDocPtr doc) {
    xmlNodePtr cur = (xmlNodePtr) doc;
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
            // <MASK>
        }
    }
}