int
tvb_get_ipv4_addr_with_prefix_len(tvbuff_t *tvb, int offset, ws_in4_addr *ipvaddress,
    guint32 prefix_len)
{
	guint8 addr_len;

	if (prefix_len > 32)
		return -1;

	addr_len = (prefix_len + 7) / 8;
	*ipvaddress = 0;
	tvb_memcpy(tvb, ipvaddress, offset, addr_len);
	if (prefix_len % 8)
		((guint8*)ipvaddress)[addr_len - 1] &= ((0xff00 >> (prefix_len % 8)) & 0xff);
	return addr_len;
}