static int
xsltCompMatchAdd(xsltParserContextPtr parserCtx, xsltCompMatchPtr comp,
                 xsltOp op, xmlChar * value, xmlChar * value2, int novar)
{
    if (comp->nbStep >= comp->maxStep) {
        xsltStepOpPtr tmp;

	tmp = (xsltStepOpPtr) xmlRealloc(comp->steps, comp->maxStep * 2 *
	                                 sizeof(xsltStepOp));
	if (tmp == NULL) {
	    xsltGenericError(xsltGenericErrorContext,
	     "xsltCompMatchAdd: memory re-allocation failure.\n");
	    if (parserCtx->style != NULL)
		parserCtx->style->errors++;
	    return (-1);
	}
        comp->maxStep *= 2;
	comp->steps = tmp;
    }
    comp->steps[comp->nbStep].op = op;
    comp->steps[comp->nbStep].value = value;
    comp->steps[comp->nbStep].value2 = value2;
    comp->steps[comp->nbStep].value3 = NULL;
    comp->steps[comp->nbStep].comp = NULL;
    if (parserCtx->ctxt != NULL) {
	comp->steps[comp->nbStep].previousExtra =
	    xsltAllocateExtraCtxt(parserCtx->ctxt);
	comp->steps[comp->nbStep].indexExtra =
	    xsltAllocateExtraCtxt(parserCtx->ctxt);
	comp->steps[comp->nbStep].lenExtra =
	    xsltAllocateExtraCtxt(parserCtx->ctxt);
    } else {
	comp->steps[comp->nbStep].previousExtra =
	    xsltAllocateExtra(parserCtx->style);
	comp->steps[comp->nbStep].indexExtra =
	    xsltAllocateExtra(parserCtx->style);
	comp->steps[comp->nbStep].lenExtra =
	    xsltAllocateExtra(parserCtx->style);
    }
    if (op == XSLT_OP_PREDICATE) {
        int flags = 0;

#ifdef XML_XPATH_NOVAR
	if (novar != 0)
	    flags = XML_XPATH_NOVAR;
#endif
	comp->steps[comp->nbStep].comp = xsltXPathCompileFlags(parserCtx->style,
                value, flags);
	if (comp->steps[comp->nbStep].comp == NULL) {
	    xsltTransformError(NULL, parserCtx->style, parserCtx->elem,
		    "Failed to compile predicate\n");
	    if (parserCtx->style != NULL)
		parserCtx->style->errors++;
	}
    }
    comp->nbStep++;
    return (0);
}