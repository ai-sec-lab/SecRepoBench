*addr = 0;
	tvb_memcpy(tvb, addr, offset, addr_len);
	if (prefix_len % 8)
		addr[addr_len - 1] &= ((0xff00 >> (prefix_len % 8)) & 0xff);
	return addr_len;