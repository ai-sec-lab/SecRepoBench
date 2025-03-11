static int
xmlXIncludeDoProcess(xmlXIncludeCtxtPtr include_context, xmlNodePtr tree) {
    xmlXIncludeRefPtr ref;
    xmlNodePtr cur;
    int ret = 0;
    int i;

    if ((tree == NULL) || (tree->type == XML_NAMESPACE_DECL))
	return(-1);
    if (include_context == NULL)
	return(-1);

    /*
     * First phase: lookup the elements in the document
     */
    cur = tree;
    do {
	/* TODO: need to work on entities -> stack */
        if (xmlXIncludeTestNode(include_context, cur) == 1) {
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            /*
             * Avoid superlinear expansion by limiting the total number
             * of replacements.
             */
            if (include_context->incTotal >= 20)
                break;
#endif
            include_context->incTotal++;
            ref = xmlXIncludeExpandNode(include_context, cur);
            /*
             * Mark direct includes.
             */
            if (ref != NULL)
                ref->replace = 1;
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
    for (i = 0; i < include_context->incNr; i++) {
	if (include_context->incTab[i]->replace != 0) {
            if ((include_context->incTab[i]->inc != NULL) ||
                (include_context->incTab[i]->emptyFb != 0)) {	/* (empty fallback) */
                xmlXIncludeIncludeNode(include_context, include_context->incTab[i]);
                include_context->incTab[i]->replace = 0;
            }
        } else {
            /*
             * Ignore includes which were added indirectly, for example
             * inside xi:fallback elements.
             */
            if (include_context->incTab[i]->inc != NULL) {
                xmlFreeNodeList(include_context->incTab[i]->inc);
                include_context->incTab[i]->inc = NULL;
            }
        }
	ret++;
    }

    return(ret);
}