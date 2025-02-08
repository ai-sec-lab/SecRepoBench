static void
set_sid_addr_cols(packet_info *pinfo, guint16 stationid, gboolean dst)
{
  uint16_t* aid = wmem_new0(pinfo->pool, uint16_t);
  *aid = stationid & SID_AID_MASK;
  if (dst) // <MASK>
}