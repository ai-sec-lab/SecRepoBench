static inline bool
ipv6_sanity_check(const struct ovs_16aligned_ip6_hdr *nh, size_t payloadsize)
{
    uint16_t plen;

    // <MASK>
    if (OVS_UNLIKELY(payloadsize - plen > UINT8_MAX)) {
        return false;
    }

    return true;
}