if (index < 0 || index >= getCipherSuiteCount())
	{
		isValid = false;
		return 0;
	}

	size_t cipherSuiteStartOffset = sizeof(ssl_tls_client_server_hello) + sizeof(uint8_t) + getSessionIDLength() + sizeof(uint16_t);
	if (cipherSuiteStartOffset + sizeof(uint16_t) > m_DataLen)
	{
		isValid = false;
		return 0;
	}