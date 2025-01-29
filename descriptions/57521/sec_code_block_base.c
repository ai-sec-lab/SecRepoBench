if (nbchars < 0) {
            /* TODO: This could be an out of memory or an encoding error. */
            xmlErrInternal(ctxt,
                           "switching encoding: encoder error\n",
                           NULL);
            xmlHaltParser(ctxt);
            return (-1);
        }
        consumed = use - xmlBufUse(in->raw);
        if ((consumed > ULONG_MAX) ||
            (in->rawconsumed > ULONG_MAX - (unsigned long)consumed))
            in->rawconsumed = ULONG_MAX;
        else
	    in->rawconsumed += consumed;