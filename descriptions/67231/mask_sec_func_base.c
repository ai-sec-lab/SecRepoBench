static void
set_sid_addr_cols(packet_info *pinfo, guint16 sid, gboolean dst)
{
  uint16_t* aid = wmem_new0(pinfo->pool, uint16_t);
  *aid = sid & SID_AID_MASK;
  if (dst) // <MASK>
}