uint16_t SSLClientHelloMessage::getCipherSuiteID(int index, bool& isValid) const
{
	// <MASK>

	isValid = true;
	uint16_t* cipherSuiteStartPos = (uint16_t*)(m_Data + cipherSuiteStartOffset);
	return be16toh(*(cipherSuiteStartPos+index));
}