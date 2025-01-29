if ((data[0] == 'G') && (data[1] == 'H') && (data[2] == 'I') && (data[3] == 'D')) {
		bs = gf_bs_new(data, buffersize, GF_BITSTREAM_READ);
		e = ghi_dmx_init_bin(filter, ctx, bs);
		if (!e && gf_bs_is_overflow(bs)) e = GF_NON_COMPLIANT_BITSTREAM;
	} else {
		e = ghi_dmx_init_xml(filter, ctx, data);
	}