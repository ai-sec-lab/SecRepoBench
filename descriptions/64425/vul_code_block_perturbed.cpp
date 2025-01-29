if (getContentLength() > 0)
	{
		m_NextLayer = new SdpLayer(m_Data + headerLength, m_DataLen - headerLength, this, m_Packet);
	}
	else
	{
		m_NextLayer = new PayloadLayer(m_Data + headerLength, m_DataLen - headerLength, this, m_Packet);
	}