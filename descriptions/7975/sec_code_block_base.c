proto_item *ttl_item;
  gchar      **srv_rr_info;

  if (type == T_SRV && name[0]) {
    srv_rr_info = wmem_strsplit(wmem_packet_scope(), name, ".", 3);

    proto_tree_add_string(rr_tree, hf_dns_srv_service, tvb, offset,
                          namelen, srv_rr_info[0]);

    if (srv_rr_info[1]) {
      proto_tree_add_string(rr_tree, hf_dns_srv_proto, tvb, offset,
                            namelen, srv_rr_info[1]);

      if (srv_rr_info[2]) {
        proto_tree_add_string(rr_tree, hf_dns_srv_name, tvb, offset,
                              namelen, srv_rr_info[2]);
      }
    }
  } else {
    proto_tree_add_string(rr_tree, hf_dns_rr_name, tvb, offset, namelen, name);
  }