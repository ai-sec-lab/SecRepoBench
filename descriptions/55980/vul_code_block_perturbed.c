encoding_error:
    {
        char buffer[150];

	snprintf(buffer, 149, "Bytes: 0x%02X 0x%02X 0x%02X 0x%02X\n",
			parserContext->input->cur[0], parserContext->input->cur[1],
			parserContext->input->cur[2], parserContext->input->cur[3]);
	__xmlErrEncoding(parserContext, XML_ERR_INVALID_CHAR,
		     "Input is not proper UTF-8, indicate encoding !\n%s",
		     BAD_CAST buffer, NULL);
    }
    return(0);