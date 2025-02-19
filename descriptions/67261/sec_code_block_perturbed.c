while (buffer[0] && strchr("\n\r\t ", (char) buffer[0])) {
		buffer++;
		res_size--;
	}