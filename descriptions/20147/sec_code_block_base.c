:
			    /* We want register + 1 when decoding T.  */
			    if (*c == 'T')
			      value = (value + 1) & 0xf;

			    if (c[1] == 'u')
			      {
				/* Eat the 'u' character.  */
				++ c;

				if (u_reg == value)
				  is_unpredictable = TRUE;
				u_reg = value;
			      }