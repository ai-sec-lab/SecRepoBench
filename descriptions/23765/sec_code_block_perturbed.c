if (ctxt->checkIndex > base) {
        base = ctxt->checkIndex;
        /* Abuse inSubset member to restore current state. */
        incomment = ctxt->inSubset & 1 ? 1 : 0;
        invalue = ctxt->inSubset & 2 ? 1 : 0;
    }

    if (in->buf == NULL) {
        buf = in->base;
        len = in->length;
    } else {
        buf = xmlBufContent(in->buf->buffer);
        len = xmlBufUse(in->buf->buffer);
    }

    /* take into account the sequence length */
    if (third)
        len -= 2;
    else if (next)
        len--;
    for (; base < len; base++) {
        if ((!incomment) && (base + 4 < len) && (!iscomment)) {
            if ((buf[base] == '<') && (buf[base + 1] == '!') &&
                (buf[base + 2] == '-') && (buf[base + 3] == '-')) {
                incomment = 1;
                /* do not increment past <! - some people use <!--> */
                base += 2;
            }
        }
        if (ignoreAttrValue) {
            if (buf[base] == '"' || buf[base] == '\'') {
                if (invalue) {
                    if (buf[base] == valdellim) {
                        invalue = 0;
                        continue;
                    }
                } else {
                    valdellim = buf[base];
                    invalue = 1;
                    continue;
                }
            } else if (invalue) {
                continue;
            }
        }
        if (incomment) {
            if (base + 3 > len)
                return (-1);
            if ((buf[base] == '-') && (buf[base + 1] == '-') &&
                (buf[base + 2] == '>')) {
                incomment = 0;
                base += 2;
            }
            continue;
        }
        if (buf[base] == first) {
            if (third != 0) {
                if ((buf[base + 1] != next) || (buf[base + 2] != third))
                    continue;
            } else if (next != 0) {
                if (buf[base + 1] != next)
                    continue;
            }
            ctxt->checkIndex = 0;
#ifdef DEBUG_PUSH
            if (next == 0)
                xmlGenericError(xmlGenericErrorContext,
                                "HPP: lookup '%c' found at %d\n",
                                first, base);
            else if (third == 0)
                xmlGenericError(xmlGenericErrorContext,
                                "HPP: lookup '%c%c' found at %d\n",
                                first, next, base);
            else
                xmlGenericError(xmlGenericErrorContext,
                                "HPP: lookup '%c%c%c' found at %d\n",
                                first, next, third, base);
#endif
            return (base - (in->cur - in->base));
        }
    }
    ctxt->checkIndex = base;
    /* Abuse inSubset member to track current state. */
    ctxt->inSubset = 0;
    if (incomment)
        ctxt->inSubset |= 1;
    if (invalue)
        ctxt->inSubset |= 2;