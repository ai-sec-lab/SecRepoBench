if (nbchars < 0) {
            xmlErrInternal(ctxt,
                           "switching encoding: encoder error\n",
                           NULL);
            return (-1);
        }
        consumed = use - xmlBufUse(in->raw);
        if ((consumed > ULONG_MAX) ||
            (in->rawconsumed > ULONG_MAX - (unsigned long)consumed))
            in->rawconsumed = ULONG_MAX;
        else
	    in->rawconsumed += consumed;