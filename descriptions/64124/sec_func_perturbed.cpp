uint16_t SSLClientHelloMessage::getCipherSuiteID(int index, bool& validFlag) const
{
	if (index < 0 || index >= getCipherSuiteCount())
	{
		validFlag = false;
		return 0;
	}

	size_t cipherSuiteStartOffset = sizeof(ssl_tls_client_server_hello) + sizeof(uint8_t) + getSessionIDLength() + sizeof(uint16_t);
	if (cipherSuiteStartOffset + sizeof(uint16_t) * (index + 1) > m_DataLen)
	{
		validFlag = false;
		return 0;
	}

	validFlag = true;
	uint16_t* cipherSuiteStartPos = (uint16_t*)(m_Data + cipherSuiteStartOffset);
	return be16toh(*(cipherSuiteStartPos+index));
}