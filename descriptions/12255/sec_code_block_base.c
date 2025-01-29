if (nl_attr_oversized(key->size - encap - NLA_HDRLEN)) {
                return -E2BIG;
            }
            s += retval;