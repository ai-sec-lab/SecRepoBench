void
xmlSAX2CDataBlock(void *ctx, const xmlChar *value, int len)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    xmlNodePtr ret, lastChild;

    if (ctx == NULL) return;
#ifdef DEBUG_SAX
    xmlGenericError(xmlGenericErrorContext,
	    "SAX.pcdata(%.10s, %d)\n", value, len);
#endif
    lastChild = xmlGetLastChild(ctxt->node);
#ifdef DEBUG_SAX_TREE
    xmlGenericError(xmlGenericErrorContext,
	    "add chars to %s \n", ctxt->node->name);
#endif
    if ((lastChild != NULL) &&
        (lastChild->type == XML_CDATA_SECTION_NODE)) {
	xmlTextConcat(lastChild, value, len);
    } else {
	// <MASK>
    }
}