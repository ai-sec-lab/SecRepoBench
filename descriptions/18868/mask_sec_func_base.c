static int
coolkey_get_life_cycle(sc_card_t *card, coolkey_life_cycle_t *life_cycle)
{
	coolkey_status_t status;
	u8 *receive_buf;
	// <MASK>
	life_cycle->protocol_version_major = status.protocol_version_major;
	life_cycle->protocol_version_minor = status.protocol_version_minor;
	life_cycle->pin_count = status.pin_count;
	return SC_SUCCESS;
}