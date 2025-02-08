xt->sax != NULL) &&
                (ctxt->disableSAX == 0) &&
		(ctxt->sax->comment != NULL)) {
		if (buf == NULL) {
		    if ((*in == '-') && (in[1] == '-'))
		        size = nbchar + 1;
		    else
		        size = XML_PARSER_BUFFER_SIZE + nbchar;
		    buf = (xmlChar *) xmlMallocAtomic(size);
		    if (buf == NULL) {
		        xmlErrMemory(ctxt, NULL);
			ctxt->instate = state;
			return;
		    }
		    len = 0;
		} else if (len + nbchar + 1 >= size) {
		    xmlChar *new_buf;
		    size  += len + nbchar + XML_PARSER_BUFFER_SIZE;
		    new_buf = (xmlChar *) xmlRealloc(buf, size);
		    if (new_buf == NULL) {
		        xmlFree (buf);
			xmlErrMemory(ctxt, NULL);
			ctxt->instate = state;
			return;
		    }
		    buf = new_buf;
		}
		memcpy(&buf[len], ctxt->input->cur, nbchar);
		len += nbchar;
		buf[len] = 0;
	    }