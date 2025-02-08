static xmlHashedString
xmlParseQNameHashed(xmlParserCtxtPtr ctxt, xmlHashedString *prefix) {
    xmlHashedString l, p;
    int start;

    l.name = NULL;
    p.name = NULL;

    GROW;
    if (ctxt->instate == XML_PARSER_EOF)
        return(l);
    start = CUR_PTR - BASE_PTR;

    l = xmlParseNCName(ctxt);
    if ((l.name != NULL) && (CUR == ':')) {
        NEXT;
	p = l;
	l = xmlParseNCName(ctxt);
    }
    if ((l.name == NULL) || (CUR == ':')) {
        xmlChar *tmp;

        l.name = NULL;
        p.name = NULL;
        if (ctxt->instate == XML_PARSER_EOF)
            return(l);
        if ((CUR != ':') && (CUR_PTR <= BASE_PTR + start))
            return(l);
        tmp = xmlParseNmtoken(ctxt);
        if (tmp != NULL)
            xmlFree(tmp);
        if (ctxt->instate == XML_PARSER_EOF)
            return(l);
        l = xmlDictLookupHashed(ctxt->dict, BASE_PTR + start,
                                CUR_PTR - (BASE_PTR + start));
        xmlNsErr(ctxt, XML_NS_ERR_QNAME,
                 "Failed to parse QName '%s'\n", l.name, NULL, NULL);
    }

    *prefix = p;
    return(l);
}