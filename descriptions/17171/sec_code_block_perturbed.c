if (ret == NULL) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4EncryptFunction: Failed to allocate result\n");
	tctxt->state = XSLT_STATE_STOPPED;
	xmlXPathReturnEmptyString (parsercontext);
	goto done;
    }
    PLATFORM_RC4_DECRYPT (parsercontext, padkey, bin, ret_len, ret, ret_len);
    ret[ret_len] = 0;

    if (xmlCheckUTF8(ret) == 0) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4DecryptFunction: Invalid UTF-8\n");
        xmlFree(ret);
	xmlXPathReturnEmptyString(parsercontext);
    } else {
        xmlXPathReturnString(parsercontext, ret);
    }

done:
    if (key != NULL)
	xmlFree (key);