if (tmp == NULL) {
	    xsltGenericError(xsltGenericErrorContext,
	     "xsltCompMatchAdd: memory re-allocation failure.\n");
	    if (parserCtx->style != NULL)
		parserCtx->style->errors++;
	    if (value)
	        xmlFree(value);
	    if (value2)
	        xmlFree(value2);
	    return (-1);
	}
        comp->maxStep *= 2;
	comp->steps = tmp;