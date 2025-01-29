std::string contentType;
	if (getContentLength() > 0)
	{
		HeaderField *contentTypeField = getFieldByName(PCPP_SIP_CONTENT_TYPE_FIELD);
		if (contentTypeField != nullptr)
			contentType = contentTypeField->getFieldValue();
	}

	if (contentType.find("application/sdp") != std::string::npos)
	{
		m_NextLayer = new SdpLayer(m_Data + headerLen, m_DataLen - headerLen, this, m_Packet);
	}
	else
	{
		m_NextLayer = new PayloadLayer(m_Data + headerLen, m_DataLen - headerLen, this, m_Packet);
	}