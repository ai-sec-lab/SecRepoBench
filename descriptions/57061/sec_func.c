static int
xsltCompMatchAdd(xsltParserContextPtr ctxt, xsltCompMatchPtr comp,
                 xsltOp op, xmlChar * value, xmlChar * value2, int novar)
{
    if (comp->nbStep >= comp->maxStep) {
        xsltStepOpPtr tmp;

	tmp = (xsltStepOpPtr) xmlRealloc(comp->steps, comp->maxStep * 2 *
	                                 sizeof(xsltStepOp));
	if (tmp == NULL) {
	    xsltGenericError(xsltGenericErrorContext,
	     "xsltCompMatchAdd: memory re-allocation failure.\n");
	    if (ctxt->style != NULL)
		ctxt->style->errors++;
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
    if (ctxt->ctxt != NULL) {
	comp->steps[comp->nbStep].previousExtra =
	    xsltAllocateExtraCtxt(ctxt->ctxt);
	comp->steps[comp->nbStep].indexExtra =
	    xsltAllocateExtraCtxt(ctxt->ctxt);
	comp->steps[comp->nbStep].lenExtra =
	    xsltAllocateExtraCtxt(ctxt->ctxt);
    } else {
	comp->steps[comp->nbStep].previousExtra =
	    xsltAllocateExtra(ctxt->style);
	comp->steps[comp->nbStep].indexExtra =
	    xsltAllocateExtra(ctxt->style);
	comp->steps[comp->nbStep].lenExtra =
	    xsltAllocateExtra(ctxt->style);
    }
    if (op == XSLT_OP_PREDICATE) {
        int flags = 0;

#ifdef XML_XPATH_NOVAR
	if (novar != 0)
	    flags = XML_XPATH_NOVAR;
#endif
	comp->steps[comp->nbStep].comp = xsltXPathCompileFlags(ctxt->style,
                value, flags);
	if (comp->steps[comp->nbStep].comp == NULL) {
	    xsltTransformError(NULL, ctxt->style, ctxt->elem,
		    "Failed to compile predicate\n");
	    if (ctxt->style != NULL)
		ctxt->style->errors++;
	}
    }
    comp->nbStep++;
    return (0);
}