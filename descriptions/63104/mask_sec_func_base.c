static int entersafe_gen_key(sc_card_t *card, sc_entersafe_gen_key_data *data)
{
	int	r;
	size_t len = data->key_length >> 3;
	sc_apdu_t apdu;
	// <MASK>
}