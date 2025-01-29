{
                size_t ignoreDepth = 0;

                SKIP(6);
                SKIP_BLANKS;
                if (RAW != '[') {
                    xmlFatalErr(ctxt, XML_ERR_CONDSEC_INVALID, NULL);
                    xmlHaltParser(ctxt);
                    goto error;
                }
                if (ctxt->input->id != id) {
                    xmlFatalErrMsg(ctxt, XML_ERR_ENTITY_BOUNDARY,
                                   "All markup of the conditional section is"
                                   " not in the same entity\n");
                }
                NEXT;

                while ((PARSER_STOPPED(ctxt) == 0) && (RAW != 0)) {
                    if ((RAW == '<') && (NXT(1) == '!') && (NXT(2) == '[')) {
                        SKIP(3);
                        ignoreDepth++;
                        /* Check for integer overflow */
                        if (ignoreDepth == 0) {
                            xmlErrMemory(ctxt, NULL);
                            goto error;
                        }
                    } else if ((RAW == ']') && (NXT(1) == ']') &&
                               (NXT(2) == '>')) {
                        if (ignoreDepth == 0)
                            break;
                        SKIP(3);
                        ignoreDepth--;
                    } else {
                        NEXT;
                    }
                }

		if (RAW == 0) {
		    xmlFatalErr(ctxt, XML_ERR_CONDSEC_NOT_FINISHED, NULL);
                    goto error;
		}
                if (ctxt->input->id != id) {
                    xmlFatalErrMsg(ctxt, XML_ERR_ENTITY_BOUNDARY,
                                   "All markup of the conditional section is"
                                   " not in the same entity\n");
                }
                SKIP(3);
            } else {
                xmlFatalErr(ctxt, XML_ERR_CONDSEC_INVALID_KEYWORD, NULL);
                xmlHaltParser(ctxt);
                goto error;
            }