while (data[0] && strchr("\n\r\t ", (char) data[0])) {
		data++;
		res_size--;
	}