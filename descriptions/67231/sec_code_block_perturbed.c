{
    set_address(&pinfo->dl_dst, wlan_aid_address_type, (int)sizeof(*aid), aid);
    copy_address_shallow(&pinfo->dst, &pinfo->dl_dst);
  } else {
    set_address(&pinfo->dl_src, wlan_aid_address_type, (int)sizeof(*aid), aid);
    copy_address_shallow(&pinfo->src, &pinfo->dl_src);
  }