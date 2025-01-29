*ipvaddress = 0;
	tvb_memcpy(tvb, ipvaddress, offset, addr_len);
	if (prefix_len % 8)
		((guint8*)ipvaddress)[addr_len - 1] &= ((0xff00 >> (prefix_len % 8)) & 0xff);
	return addr_len;