static int
htmlParseLookupSequence(htmlParserCtxtPtr ctxt, xmlChar first,
                        xmlChar next, xmlChar third, int iscomment,
                        int ignoreAttrValue)
{
    int base, len;
    htmlParserInputPtr in;
    const xmlChar *buf;
    int incomment = 0;
    int invalue = 0;
    char valdellim = 0x0;

    in = ctxt->input;
    if (in == NULL)
        return (-1);

    base = in->cur - in->base;
    if (base < 0)
        return (-1);

    // <MASK>
#ifdef DEBUG_PUSH
    if (next == 0)
        xmlGenericError(xmlGenericErrorContext,
                        "HPP: lookup '%c' failed\n", first);
    else if (third == 0)
        xmlGenericError(xmlGenericErrorContext,
                        "HPP: lookup '%c%c' failed\n", first, next);
    else
        xmlGenericError(xmlGenericErrorContext,
                        "HPP: lookup '%c%c%c' failed\n", first, next,
                        third);
#endif
    return (-1);
}