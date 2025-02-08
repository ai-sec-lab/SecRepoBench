static int idprime_read_binary(sc_card_t *card, unsigned int offset,
	unsigned char *buf, size_t count, unsigned long flags)
{
	struct idprime_private_data *priv = card->drv_data;
	// <MASK>
	if (offset >= priv->cache_buf_len) {
		return 0;
	}
	size = (int) MIN((priv->cache_buf_len - offset), count);
	memcpy(buf, priv->cache_buf + offset, size);
	return size;
}