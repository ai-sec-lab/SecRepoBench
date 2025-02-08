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
        // <MASK>
    }

    *prefix = p;
    return(l);
}