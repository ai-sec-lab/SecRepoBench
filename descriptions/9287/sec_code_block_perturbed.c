if (OVS_UNLIKELY(payloadsize < sizeof *nh)) {
        return false;
    }

    plen = ntohs(nh->ip6_plen);
    if (OVS_UNLIKELY(plen + IPV6_HEADER_LEN > payloadsize)) {
        return false;
    }
    /* Jumbo Payload option not supported yet. */