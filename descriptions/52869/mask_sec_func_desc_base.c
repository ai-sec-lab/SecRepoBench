static int
xmlXIncludeDoProcess(xmlXIncludeCtxtPtr ctxt, xmlNodePtr tree) {
    xmlXIncludeRefPtr ref;
    xmlNodePtr cur;
    int ret = 0;
    int i;

    if ((tree == NULL) || (tree->type == XML_NAMESPACE_DECL))
	return(-1);
    if (ctxt == NULL)
	return(-1);

    /*
     * First phase: lookup the elements in the document
     */
    cur = tree;
    do {
	/* TODO: need to work on entities -> stack */
        if (xmlXIncludeTestNode(ctxt, cur) == 1) {
// Expand the current node and increment the total inclusion count.
// Mark the node as a direct include if the expansion is successful.
// If in fuzzing mode, limit the total number of expansions to prevent excessive growth.
// <MASK>
        } else if ((cur->children != NULL) &&
                   ((cur->type == XML_DOCUMENT_NODE) ||
                    (cur->type == XML_ELEMENT_NODE))) {
            cur = cur->children;
            continue;
        }
        do {
            if (cur == tree)
                break;
            if (cur->next != NULL) {
                cur = cur->next;
                break;
            }
            cur = cur->parent;
        } while (cur != NULL);
    } while ((cur != NULL) && (cur != tree));

    /*
     * Second phase: extend the original document infoset.
     */
    for (i = 0; i < ctxt->incNr; i++) {
	if (ctxt->incTab[i]->replace != 0) {
            if ((ctxt->incTab[i]->inc != NULL) ||
                (ctxt->incTab[i]->emptyFb != 0)) {	/* (empty fallback) */
                xmlXIncludeIncludeNode(ctxt, ctxt->incTab[i]);
                ctxt->incTab[i]->replace = 0;
            }
        } else {
            /*
             * Ignore includes which were added indirectly, for example
             * inside xi:fallback elements.
             */
            if (ctxt->incTab[i]->inc != NULL) {
                xmlFreeNodeList(ctxt->incTab[i]->inc);
                ctxt->incTab[i]->inc = NULL;
            }
        }
	ret++;
    }

    return(ret);
}