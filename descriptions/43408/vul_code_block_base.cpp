char* data = (char*)(m_SipRequest->m_Data + m_UriOffset);
	char* verPos = (char*)cross_platform_memmem(data, m_SipRequest->getDataLen() - m_UriOffset, " SIP/", 5);
	if (verPos == NULL)
	{
		m_Version = "";
		m_VersionOffset = -1;
		return;
	}

	// verify packet doesn't end before the version, meaning still left place for " SIP/x.y" (7 chars)
	if ((uint16_t)(verPos + 7 - (char*)m_SipRequest->m_Data) > m_SipRequest->getDataLen())
	{
		m_Version = "";
		m_VersionOffset = -1;
		return;
	}

	//skip the space char
	verPos++;

	int endOfVerPos = 0;
	while (((verPos + endOfVerPos) < (char *) (m_SipRequest->m_Data + m_SipRequest->m_DataLen)) && ((verPos+endOfVerPos)[0] != '\r') && ((verPos+endOfVerPos)[0] != '\n'))
		endOfVerPos++;

	m_Version = std::string(verPos, endOfVerPos);

	m_VersionOffset = verPos - (char*)m_SipRequest->m_Data;