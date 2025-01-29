if (xmlCheckUTF8(ret) == 0) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4DecryptFunction: Invalid UTF-8\n");
        xmlFree(ret);
	xmlXPathReturnEmptyString(ctxt);
    } else {
        xmlXPathReturnString(ctxt, ret);
    }

done:
    if (key != NULL)
	xmlFree (key);
    if (str != NULL)
	xmlFree (str);
    if (padkey != NULL)
	xmlFree (padkey);
    if (bin != NULL)
	xmlFree (bin);