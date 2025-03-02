ETH_P_MPLS_MULTI:
    if(ip_offset+4 >= (int)header->caplen)
      return(nproto);
    mpls.u32 = *((uint32_t *) &packet[ip_offset]);
    mpls.u32 = ntohl(mpls.u32);
    workflow->stats.mpls_count++;
    type = ETH_P_IP, ip_offset += 4;

    while(!mpls.mpls.s && (((bpf_u_int32)ip_offset) + 4 < header->caplen)) {
      mpls.u32 = *((uint32_t *) &packet[ip_offset]);
      mpls.u32 = ntohl(mpls.u32);
      ip_offset += 4;
    }
    recheck_type = 1;
    break;