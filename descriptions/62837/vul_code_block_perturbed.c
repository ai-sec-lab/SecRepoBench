xmlChar *tmp;

        p.name = NULL;
        if (parserContext->instate == XML_PARSER_EOF)
            return(l);
        if ((CUR != ':') && (CUR_PTR <= BASE_PTR + start))
            return(l);
        tmp = xmlParseNmtoken(parserContext);
        if (tmp != NULL)
            xmlFree(tmp);
        if (parserContext->instate == XML_PARSER_EOF)
            return(l);
        l = xmlDictLookupHashed(parserContext->dict, BASE_PTR + start,
                                CUR_PTR - (BASE_PTR + start));
        xmlNsErr(parserContext, XML_NS_ERR_QNAME,
                 "Failed to parse QName '%s'\n", l.name, NULL, NULL);