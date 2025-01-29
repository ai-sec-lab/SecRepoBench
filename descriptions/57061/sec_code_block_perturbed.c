if (tmp == NULL) {
	    xsltGenericError(xsltGenericErrorContext,
	     "xsltCompMatchAdd: memory re-allocation failure.\n");
	    if (parserCtx->style != NULL)
		parserCtx->style->errors++;
	    return (-1);
	}
        comp->maxStep *= 2;
	comp->steps = tmp;