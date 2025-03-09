static void
exsltCryptoRc4DecryptFunction (xmlXPathParserContextPtr parsercontext, int nargs) {

    int key_len = 0;
    int str_len = 0, bin_len = 0, ret_len = 0;
    xmlChar *key = NULL, *str = NULL, *padkey = NULL, *bin =
	NULL, *ret = NULL;
    xsltTransformContextPtr tctxt = NULL;

    if (nargs != 2) {
	xmlXPathSetArityError (parsercontext);
	return;
    }
    tctxt = xsltXPathGetTransformContext(parsercontext);

    str = xmlXPathPopString (parsercontext);
    str_len = xmlStrlen (str);

    if (str_len == 0) {
	xmlXPathReturnEmptyString (parsercontext);
	xmlFree (str);
	return;
    }

    key = xmlXPathPopString (parsercontext);
    key_len = xmlStrlen (key);

    if (key_len == 0) {
	xmlXPathReturnEmptyString (parsercontext);
	xmlFree (key);
	xmlFree (str);
	return;
    }

    padkey = xmlMallocAtomic (RC4_KEY_LENGTH + 1);
    if (padkey == NULL) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4EncryptFunction: Failed to allocate padkey\n");
	tctxt->state = XSLT_STATE_STOPPED;
	xmlXPathReturnEmptyString (parsercontext);
	goto done;
    }
    memset(padkey, 0, RC4_KEY_LENGTH + 1);
    if ((key_len > RC4_KEY_LENGTH) || (key_len < 0)) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4EncryptFunction: key size too long or key broken\n");
	tctxt->state = XSLT_STATE_STOPPED;
	xmlXPathReturnEmptyString (parsercontext);
	goto done;
    }
    memcpy (padkey, key, key_len);

/* decode hex to binary */
    bin_len = str_len;
    bin = xmlMallocAtomic (bin_len);
    if (bin == NULL) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4EncryptFunction: Failed to allocate string\n");
	tctxt->state = XSLT_STATE_STOPPED;
	xmlXPathReturnEmptyString (parsercontext);
	goto done;
    }
    ret_len = exsltCryptoHex2Bin (str, str_len, bin, bin_len);

/* decrypt the binary blob */
    ret = xmlMallocAtomic (ret_len + 1);
    if (ret == NULL) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4EncryptFunction: Failed to allocate result\n");
	tctxt->state = XSLT_STATE_STOPPED;
	xmlXPathReturnEmptyString (parsercontext);
	goto done;
    }
    PLATFORM_RC4_DECRYPT (parsercontext, padkey, bin, ret_len, ret, ret_len);
    ret[ret_len] = 0;

    // return the decrypted string
    // <MASK>

done:
    if (key != NULL)
	xmlFree (key);
    if (str != NULL)
	xmlFree (str);
    if (padkey != NULL)
	xmlFree (padkey);
    if (bin != NULL)
	xmlFree (bin);
}