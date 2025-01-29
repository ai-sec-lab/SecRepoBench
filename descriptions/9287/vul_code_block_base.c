if (OVS_UNLIKELY(size < sizeof *nh)) {
        return false;
    }

    plen = ntohs(nh->ip6_plen);
    if (OVS_UNLIKELY(plen > size)) {
        return false;
    }
    /* Jumbo Payload option not supported yet. */