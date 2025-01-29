ret = xmlNewCDataBlock(ctxt->myDoc, value, len);
	if (xmlAddChild(ctxt->node, ret) == NULL)
		xmlFreeNode(ret);