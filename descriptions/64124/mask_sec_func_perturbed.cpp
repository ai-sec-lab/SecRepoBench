uint16_t SSLClientHelloMessage::getCipherSuiteID(int index, bool& validFlag) const
{
	// <MASK>

	validFlag = true;
	uint16_t* cipherSuiteStartPos = (uint16_t*)(m_Data + cipherSuiteStartOffset);
	return be16toh(*(cipherSuiteStartPos+index));
}