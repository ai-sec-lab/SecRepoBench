static void
add_rr_to_tree(proto_tree  *rr_tree, tvbuff_t *tvb, int offset,
  const guchar *name, int namelength, int type,
  packet_info *pinfo, gboolean is_mdns)
{
  // <MASK>

  offset += namelength;

  proto_tree_add_item(rr_tree, hf_dns_rr_type, tvb, offset, 2, ENC_BIG_ENDIAN);
  offset += 2;
  if (is_mdns) {
    proto_tree_add_item(rr_tree, hf_dns_rr_class_mdns, tvb, offset, 2, ENC_BIG_ENDIAN);
    proto_tree_add_item(rr_tree, hf_dns_rr_cache_flush, tvb, offset, 2, ENC_BIG_ENDIAN);
  } else {
    proto_tree_add_item(rr_tree, hf_dns_rr_class, tvb, offset, 2, ENC_BIG_ENDIAN);
  }
  offset += 2;
  ttl_item = proto_tree_add_item(rr_tree, hf_dns_rr_ttl, tvb, offset, 4, ENC_BIG_ENDIAN);
  if (tvb_get_ntohl(tvb, offset) & 0x80000000) {
    expert_add_info(pinfo, ttl_item, &ei_ttl_negative);
  }

  offset += 4;
  proto_tree_add_item(rr_tree, hf_dns_rr_len, tvb, offset, 2, ENC_BIG_ENDIAN);
}