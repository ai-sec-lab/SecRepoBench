static int
cflex_delete_file(sc_profile_t *profile, sc_pkcs15_card_t *p15card, sc_file_t *df)
{
        sc_path_t  path;
        sc_file_t  *parent;
        int             r = 0;
        /* Select the parent DF */
        path = df->path;
		// <MASK>
}