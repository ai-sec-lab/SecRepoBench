ret = xmlNewCDataBlock(ctxt->myDoc, cdataContent, len);
	if (xmlAddChild(ctxt->node, ret) == NULL)
		xmlFreeNode(ret);