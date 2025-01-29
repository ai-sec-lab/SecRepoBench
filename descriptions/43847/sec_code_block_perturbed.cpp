if (resultPtr - result < 256)
	{
		// add the last '\0' to encodedNameLength
		resultPtr[0] = 0;
		encodedNameLength++;
	}

	return encodedNameLength;