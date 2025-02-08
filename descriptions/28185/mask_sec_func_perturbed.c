static int idprime_get_token_name(sc_card_t* cardcontext, char** tname)
{
	idprime_private_data_t * priv = cardcontext->drv_data;
	sc_path_t tinfo_path = {"\x00\x00", 2, 0, 0, SC_PATH_TYPE_PATH, {"", 0}};
	sc_file_t *file = NULL;
	u8 buf[2];
	// <MASK>

	LOG_FUNC_RETURN(cardcontext->ctx, SC_SUCCESS);
}