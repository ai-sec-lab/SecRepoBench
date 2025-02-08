static int
coolkey_get_life_cycle(sc_card_t *card, coolkey_life_cycle_t *lifecyclestate)
{
	coolkey_status_t status;
	u8 *receive_buf;
	// <MASK>
	lifecyclestate->protocol_version_major = status.protocol_version_major;
	lifecyclestate->protocol_version_minor = status.protocol_version_minor;
	lifecyclestate->pin_count = status.pin_count;
	return SC_SUCCESS;
}