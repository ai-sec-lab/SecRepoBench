xmlXPathReturnString (ctxt, ret);

done:
    if (key != NULL)
	xmlFree (key);
    if (str != NULL)
	xmlFree (str);
    if (padkey != NULL)
	xmlFree (padkey);
    if (bin != NULL)
	xmlFree (bin);