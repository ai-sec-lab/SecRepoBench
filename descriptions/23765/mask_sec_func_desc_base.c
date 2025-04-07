static int
htmlParseLookupSequence(htmlParserCtxtPtr ctxt, xmlChar first,
                        xmlChar next, xmlChar third, int iscomment,
                        int ignoreattrval)
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

    // Determine the starting position for parsing based on the parser context.
    // Obtain a pointer to the input buffer and its length.
    // Adjust the length to account for the number of characters in the search sequence.
    // Iterate through the input buffer starting from the current position.
    // Check for the beginning of comment sections and set flags accordingly.
    // Skip content within attribute values if ignoring attribute values is enabled.
    // Within comments, search for the end of the comment and adjust the position accordingly.
    // Check if the current character matches the first character of the sequence being searched.
    // If the subsequent characters match the second and third (if provided), return the position.
    // Update the context's check index based on the current position after iteration.
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