char buffer[16];

      len_avail -= 1 + strlen (version_string);

      if (sym_info == symbol_undefined)
	len_avail -= sprintf (buffer," (%d)", vna_other);
      else if (sym_info != symbol_hidden)
	len_avail -= 1;