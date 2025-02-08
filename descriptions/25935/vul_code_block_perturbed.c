if(i > file_size)
			return -1;

		field_size = hextoint((char*) &file[i], 2);
		if((field_size < 0) || (field_size+i > file_size))
			return -1;

		i += 2;

		if(field_size >= (int)sizeof(fields[f].value))
			return -1;