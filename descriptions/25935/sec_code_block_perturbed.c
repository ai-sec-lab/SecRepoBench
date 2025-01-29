if(i+2 > file_size)
			return -1;
		field_size = hextoint((char*) &file[i], 2);
		i += 2;

		if (field_size < 0
				|| i + field_size > file_size
				|| field_size >= (int)sizeof(fields[f].value))
			return -1;