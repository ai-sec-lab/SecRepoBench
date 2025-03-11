u32 gf_mp3_bit_rate(u32 hdr)
{
	u8 version = gf_mp3_version(hdr);
	u8 layer = gf_mp3_layer(hdr);
	u8 bitrateIdx = (hdr >> 12) & 0xF;
	u32 lidx;
	/*MPEG-1*/
	// <MASK>
}