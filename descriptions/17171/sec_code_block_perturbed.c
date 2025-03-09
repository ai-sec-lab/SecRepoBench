if (xmlCheckUTF8(ret) == 0) {
	xsltTransformError(tctxt, NULL, tctxt->inst,
	    "exsltCryptoRc4DecryptFunction: Invalid UTF-8\n");
        xmlFree(ret);
	xmlXPathReturnEmptyString(parsercontext);
    } else {
        xmlXPathReturnString(parsercontext, ret);
    }