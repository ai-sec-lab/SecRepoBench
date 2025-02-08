int ndpi_add_ip_risk_mask(struct ndpi_detection_module_struct *ndpi_str,
			  char *ip, ndpi_risk mask) {
  char *cidr, *saveptr, *addr = strtok_r(ip, "/", &saveptr);
  int is_ipv6 = 0;
  ndpi_patricia_node_t *node = NULL;

  if(!addr || strlen(addr) == 0)
    return(-2);

  if(ip[0] == '[') {
    is_ipv6 = 1;
    addr += 1;
    addr[strlen(addr) - 1] = '\0'; /* strip ']' */
  }

  cidr = strtok_r(NULL, "\n", &saveptr);

  if(!is_ipv6 && ndpi_str->ip_risk_mask_ptree) {
    struct in_addr addrv;

    if(inet_pton(AF_INET, addr, &addrv) != 1)
      return(-1);
    node = add_to_ptree(ndpi_str->ip_risk_mask_ptree, AF_INET,
			&addrv, cidr ? atoi(cidr) : 32 /* bits */);
  } else if(is_ipv6 && ndpi_str->ip_risk_mask_ptree6) {
    struct in6_addr pin6;

    if(inet_pton(AF_INET6, addr, &pin6) != 1)
      return(-1);
    node = add_to_ptree(ndpi_str->ip_risk_mask_ptree6, AF_INET6,
			&pin6, cidr ? atoi(cidr) : 128 /* bits */);
  } else {
    return(-2);
  }

  if(node) {
    node->value.u.uv64 = (u_int64_t)mask;
    return(0);
  }
  return(-1);
}