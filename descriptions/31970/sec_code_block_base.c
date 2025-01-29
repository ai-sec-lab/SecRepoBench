(msg != NULL && data != NULL && msg->fragSz <= msg->sz &&
        fragSz <= msg->sz && fragOffset <= msg->sz &&
        (fragOffset + fragSz) <= msg->sz) {
        DtlsFrag* cur = msg->fragList;
        DtlsFrag* prev = cur;
        DtlsFrag* newFrag;
        word32 bytesLeft = fragSz; /* could be overlapping fragment */
        word32 startOffset = fragOffset;
        word32 added;

        msg->seq = seq;
        msg->epoch = epoch;
        msg->type = type;

        if (fragOffset == 0) {
            XMEMCPY(msg->buf, data - DTLS_HANDSHAKE_HEADER_SZ,
                    DTLS_HANDSHAKE_HEADER_SZ);
            c32to24(msg->sz, msg->msg - DTLS_HANDSHAKE_FRAG_SZ);
        }

        /* if no message data, just return */
        if (fragSz == 0)
            return 0;

        /* if list is empty add full fragment to front */
        if (cur == NULL) {
            newFrag = CreateFragment(&fragOffset, fragOffset + fragSz - 1, data,
                                     msg->msg, &bytesLeft, heap);
            if (newFrag == NULL)
                return MEMORY_E;

            msg->fragSz = fragSz;
            msg->fragList = newFrag;

            return 0;
        }

        /* add to front if before current front, up to next->begin */
        if (fragOffset < cur->begin) {
            word32 end = fragOffset + fragSz - 1;

            if (end >= cur->begin)
                end = cur->begin - 1;

            added = end - fragOffset + 1;
            newFrag = CreateFragment(&fragOffset, end, data, msg->msg,
                                     &bytesLeft, heap);
            if (newFrag == NULL)
                return MEMORY_E;

            msg->fragSz += added;

            newFrag->next = cur;
            msg->fragList = newFrag;
        }

        /* while we have bytes left, try to find a gap to fill */
        while (bytesLeft > 0) {
            /* get previous packet in list */
            while (cur && (fragOffset >= cur->begin)) {
                prev = cur;
                cur = cur->next;
            }

            /* don't add duplicate data */
            if (prev->end >= fragOffset) {
                if ( (fragOffset + bytesLeft - 1) <= prev->end)
                    return 0;
                fragOffset = prev->end + 1;
                bytesLeft = startOffset + fragSz - fragOffset;
            }

            if (cur == NULL)
                /* we're at the end */
                added = bytesLeft;
            else
                /* we're in between two frames */
                added = min(bytesLeft, cur->begin - fragOffset);

            /* data already there */
            if (added == 0)
                continue;

            newFrag = CreateFragment(&fragOffset, fragOffset + added - 1,
                                     data + fragOffset - startOffset,
                                     msg->msg, &bytesLeft, heap);
            if (newFrag == NULL)
                return MEMORY_E;

            msg->fragSz += added;

            newFrag->next = prev->next;
            prev->next = newFrag;
        }
    }