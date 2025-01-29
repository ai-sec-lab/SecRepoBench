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

                while (PARSER_STOPPED(ctxt) == 0) {
                    if (RAW == 0) {
                        xmlFatalErr(ctxt, XML_ERR_CONDSEC_NOT_FINISHED, NULL);
                        goto error;
                    }
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
                        SKIP(3);
                        if (ignoreDepth == 0)
                            break;
                        ignoreDepth--;
                    } else {
                        NEXT;
                    }
                }

                if (ctxt->input->id != id) {
                    xmlFatalErrMsg(ctxt, XML_ERR_ENTITY_BOUNDARY,
                                   "All markup of the conditional section is"
                                   " not in the same entity\n");
                }
            } else {
                xmlFatalErr(ctxt, XML_ERR_CONDSEC_INVALID_KEYWORD, NULL);
                xmlHaltParser(ctxt);
                goto error;
            }