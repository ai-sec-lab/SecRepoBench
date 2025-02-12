if (tmp == NULL) {
	    xsltGenericError(xsltGenericErrorContext,
	     "xsltCompMatchAdd: memory re-allocation failure.\n");
	    if (ctxt->style != NULL)
		ctxt->style->errors++;
	    if (value)
	        xmlFree(value);
	    if (value2)
	        xmlFree(value2);
	    return (-1);
	}
        comp->maxStep *= 2;
	comp->steps = tmp;