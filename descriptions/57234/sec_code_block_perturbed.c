xmlXPathCompExprPtr comp = NULL;
    xsltAttrVTPtr avt;
    int i = 0, lastavt = 0;

    if ((styleSheet == NULL) || (attr == NULL) || (attr->children == NULL))
        return;
    if ((attr->children->type != XML_TEXT_NODE) ||
        (attr->children->next != NULL)) {
        xsltTransformError(NULL, styleSheet, attr->parent,
	    "Attribute '%s': The content is expected to be a single text "
	    "node when compiling an AVT.\n", attr->name);
	styleSheet->errors++;
	return;
    }
    str = attr->children->content;
    if ((xmlStrchr(str, '{') == NULL) &&
        (xmlStrchr(str, '}') == NULL)) return;

#ifdef WITH_XSLT_DEBUG_AVT
    xsltGenericDebug(xsltGenericDebugContext,
		    "Found AVT %s: %s\n", attr->name, str);
#endif
    if (attr->psvi != NULL) {
#ifdef WITH_XSLT_DEBUG_AVT
	xsltGenericDebug(xsltGenericDebugContext,
			"AVT %s: already compiled\n", attr->name);
#endif
        return;
    }
    /*
    * Create a new AVT object.
    */
    avt = xsltNewAttrVT(styleSheet);
    if (avt == NULL)
	return;
    attr->psvi = avt;

    avt->nsList = xmlGetNsList(attr->doc, attr->parent);
    if (avt->nsList != NULL) {
	while (avt->nsList[i] != NULL)
	    i++;
    }
    avt->nsNr = i;

    cur = str;
    while (*cur != 0) {
	if (*cur == '{') {
	    if (*(cur+1) == '{') {	/* escaped '{' */
	        cur++;
		ret = xmlStrncat(ret, str, cur - str);
		cur++;
		str = cur;
		continue;
	    }
	    if (*(cur+1) == '}') {	/* skip empty AVT */
		ret = xmlStrncat(ret, str, cur - str);
	        cur += 2;
		str = cur;
		continue;
	    }
	    if ((ret != NULL) || (cur - str > 0)) {
		ret = xmlStrncat(ret, str, cur - str);
		str = cur;
		if (avt->nb_seg == 0)
		    avt->strstart = 1;
		if ((avt = xsltSetAttrVTsegment(avt, (void *) ret)) == NULL)
		    goto error;
		ret = NULL;
		lastavt = 0;
	    }

	    cur++;
	    while ((*cur != 0) && (*cur != '}')) {
		/* Need to check for literal (bug539741) */
		if ((*cur == '\'') || (*cur == '"')) {
		    char delim = *(cur++);
		    while ((*cur != 0) && (*cur != delim))
			cur++;
		    if (*cur != 0)
			cur++;	/* skip the ending delimiter */
		} else
		    cur++;
	    }
	    if (*cur == 0) {
	        xsltTransformError(NULL, styleSheet, attr->parent,
		     "Attribute '%s': The AVT has an unmatched '{'.\n",
		     attr->name);
		styleSheet->errors++;
		goto error;
	    }
	    str++;
	    expr = xmlStrndup(str, cur - str);
	    if (expr == NULL) {
		/*
		* TODO: What needs to be done here?
		*/
	        XSLT_TODO
		goto error;
	    } else {
		comp = xsltXPathCompile(styleSheet, expr);
		if (comp == NULL) {
		    xsltTransformError(NULL, styleSheet, attr->parent,
			 "Attribute '%s': Failed to compile the expression "
			 "'%s' in the AVT.\n", attr->name, expr);
		    styleSheet->errors++;
		    goto error;
		}
		if (avt->nb_seg == 0)
		    avt->strstart = 0;
		if (lastavt == 1) {
		    if ((avt = xsltSetAttrVTsegment(avt, NULL)) == NULL) {
                        xsltTransformError(NULL, styleSheet, attr->parent,
                                           "out of memory\n");
		        goto error;
                    }
		}
		if ((avt = xsltSetAttrVTsegment(avt, (void *) comp)) == NULL) {
                    xsltTransformError(NULL, styleSheet, attr->parent,
                                       "out of memory\n");
		    goto error;
                }
		lastavt = 1;
		xmlFree(expr);
		expr = NULL;
                comp = NULL;
	    }
	    cur++;
	    str = cur;
	} else if (*cur == '}') {
	    cur++;
	    if (*cur == '}') {	/* escaped '}' */
		ret = xmlStrncat(ret, str, cur - str);
		cur++;
		str = cur;
		continue;
	    } else {
	        xsltTransformError(NULL, styleSheet, attr->parent,
		     "Attribute '%s': The AVT has an unmatched '}'.\n",
		     attr->name);
		goto error;
	    }
	} else
	    cur++;
    }
    if ((ret != NULL) || (cur - str > 0)) {
	ret = xmlStrncat(ret, str, cur - str);
	str = cur;
	if (avt->nb_seg == 0)
	    avt->strstart = 1;
	if ((avt = xsltSetAttrVTsegment(avt, (void *) ret)) == NULL)
	    goto error;
	ret = NULL;
    }

error:
    if (avt == NULL) {
        xsltTransformError(NULL, styleSheet, attr->parent,
		"xsltCompileAttr: malloc problem\n");
    } else {
        if (attr->psvi != avt) {  /* may have changed from realloc */
            attr->psvi = avt;
	    /*
	     * This is a "hack", but I can't see any clean method of
	     * doing it.  If a re-alloc has taken place, then the pointer
	     * for this AVT may have changed.  style->attVTs was set by
	     * xsltNewAttrVT, so it needs to be re-set to the new value!
	     */
	    styleSheet->attVTs = avt;
	}
    }
    if (ret != NULL)
	xmlFree(ret);
    if (expr != NULL)
	xmlFree(expr);
    if (comp != NULL)
        xmlXPathFreeCompExpr(comp);