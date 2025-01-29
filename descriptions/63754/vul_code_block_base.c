{
    struct in_addr pin;

    pin.s_addr = inet_addr(addr);
    node = add_to_ptree(ndpi_str->ip_risk_mask_ptree, AF_INET,
			&pin, cidr ? atoi(cidr) : 32 /* bits */);
  } else if(is_ipv6 && ndpi_str->ip_risk_mask_ptree6) {
    struct in6_addr pin6;

    inet_pton(AF_INET6, addr, &pin6);
    node = add_to_ptree(ndpi_str->ip_risk_mask_ptree6, AF_INET6,
			&pin6, cidr ? atoi(cidr) : 128 /* bits */);
  } else {
    return(-2);
  }