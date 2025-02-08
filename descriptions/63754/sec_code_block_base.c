{
    struct in_addr pin;

    if(inet_pton(AF_INET, addr, &pin) != 1)
      return(-1);
    node = add_to_ptree(ndpi_str->ip_risk_mask_ptree, AF_INET,
			&pin, cidr ? atoi(cidr) : 32 /* bits */);
  } else if(is_ipv6 && ndpi_str->ip_risk_mask_ptree6) {
    struct in6_addr pin6;

    if(inet_pton(AF_INET6, addr, &pin6) != 1)
      return(-1);
    node = add_to_ptree(ndpi_str->ip_risk_mask_ptree6, AF_INET6,
			&pin6, cidr ? atoi(cidr) : 128 /* bits */);
  } else {
    return(-2);
  }