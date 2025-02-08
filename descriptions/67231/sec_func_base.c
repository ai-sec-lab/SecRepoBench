static void
set_sid_addr_cols(packet_info *pinfo, guint16 sid, gboolean dst)
{
  uint16_t* aid = wmem_new0(pinfo->pool, uint16_t);
  *aid = sid & SID_AID_MASK;
  if (dst) {
    set_address(&pinfo->dl_dst, wlan_aid_address_type, (int)sizeof(*aid), aid);
    copy_address_shallow(&pinfo->dst, &pinfo->dl_dst);
  } else {
    set_address(&pinfo->dl_src, wlan_aid_address_type, (int)sizeof(*aid), aid);
    copy_address_shallow(&pinfo->src, &pinfo->dl_src);
  }
}