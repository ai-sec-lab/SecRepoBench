if (oldnode == tmp)
                    oldnode = NULL;
		xmlUnlinkNode(tmp);
		xmlTextReaderFreeNode(reader, tmp);