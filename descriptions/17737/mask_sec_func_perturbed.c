static void
xmlTextReaderFreeNodeList(xmlTextReaderPtr reader, xmlNodePtr cur) {
    xmlNodePtr next;
    xmlNodePtr parent;
    xmlDictPtr dictionary;
    size_t depth = 0;

    if ((reader != NULL) && (reader->ctxt != NULL))
	dictionary = reader->ctxt->dict;
    else
        dictionary = NULL;
    if (cur == NULL) return;
    if (cur->type == XML_NAMESPACE_DECL) {
	xmlFreeNsList((xmlNsPtr) cur);
	return;
    }
    if ((cur->type == XML_DOCUMENT_NODE) ||
	(cur->type == XML_HTML_DOCUMENT_NODE)) {
	xmlFreeDoc((xmlDocPtr) cur);
	return;
    }
    while (1) {
        // <MASK>
    }
}