xmlXPathError
xsltFormatNumberConversion(xsltDecimalFormatPtr self,
			   xmlChar *fmt,
			   double number,
			   xmlChar **result)
{
    xmlXPathError status = XPATH_EXPRESSION_OK;
    xmlBufferPtr buffer;
    xmlChar *the_format, *prefix = NULL, *suffix = NULL;
    xmlChar *nprefix, *nsuffix = NULL;
    int	    prefix_length, suffix_length = 0, nprefix_length, nsuffix_length;
    // <MASK>
    number += .5 / scale;
    number -= fmod(number, 1 / scale);

    /* Next do the integer part of the number */
    if ((self->grouping != NULL) &&
        (self->grouping[0] != 0)) {
        int gchar;

	len = xmlStrlen(self->grouping);
	gchar = xsltGetUTF8Char(self->grouping, &len);
	xsltNumberFormatDecimal(buffer, floor(number), self->zeroDigit[0],
				format_info.integer_digits,
				format_info.group,
				gchar, len);
    } else
	xsltNumberFormatDecimal(buffer, floor(number), self->zeroDigit[0],
				format_info.integer_digits,
				format_info.group,
				',', 1);

    /* Special case: java treats '.#' like '.0', '.##' like '.0#', etc. */
    if ((format_info.integer_digits + format_info.integer_hash +
	 format_info.frac_digits == 0) && (format_info.frac_hash > 0)) {
        ++format_info.frac_digits;
	--format_info.frac_hash;
    }

    /* Add leading zero, if required */
    if ((floor(number) == 0) &&
	(format_info.integer_digits + format_info.frac_digits == 0)) {
        xmlBufferAdd(buffer, self->zeroDigit, xmlUTF8Strsize(self->zeroDigit, 1));
    }

    /* Next the fractional part, if required */
    if (format_info.frac_digits + format_info.frac_hash == 0) {
        if (format_info.add_decimal)
	    xmlBufferAdd(buffer, self->decimalPoint,
			 xmlUTF8Strsize(self->decimalPoint, 1));
    }
    else {
      number -= floor(number);
	if ((number != 0) || (format_info.frac_digits != 0)) {
	    xmlBufferAdd(buffer, self->decimalPoint,
			 xmlUTF8Strsize(self->decimalPoint, 1));
	    number = floor(scale * number + 0.5);
	    for (j = format_info.frac_hash; j > 0; j--) {
		if (fmod(number, 10.0) >= 1.0)
		    break; /* for */
		number /= 10.0;
	    }
	    xsltNumberFormatDecimal(buffer, floor(number), self->zeroDigit[0],
				format_info.frac_digits + j,
				0, 0, 0);
	}
    }
    /* Put the suffix into the buffer */
    for (j = 0; j < suffix_length; ) {
	if (*suffix == SYMBOL_QUOTE)
            suffix++;
        len = xmlUTF8Strsize(suffix, 1);
        xmlBufferAdd(buffer, suffix, len);
        suffix += len;
        j += len;
    }

    *result = xmlStrdup(xmlBufferContent(buffer));
    xmlBufferFree(buffer);
    return status;
}