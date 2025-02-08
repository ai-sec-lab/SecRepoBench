static inline bool
ipv6_sanity_check(const struct ovs_16aligned_ip6_hdr *nh, size_t payloadsize)
{
    uint16_t plen;

    if (OVS_UNLIKELY(payloadsize < sizeof *nh)) {
        return false;
    }

    plen = ntohs(nh->ip6_plen);
    if (OVS_UNLIKELY(plen + IPV6_HEADER_LEN > payloadsize)) {
        return false;
    }
    /* Jumbo Payload option not supported yet. */
    if (OVS_UNLIKELY(payloadsize - plen > UINT8_MAX)) {
        return false;
    }

    return true;
}